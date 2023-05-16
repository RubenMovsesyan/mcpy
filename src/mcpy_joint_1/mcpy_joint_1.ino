// MoCopy joint device (between external and central).
// Install the ArduinoBLE library from the library manager in the IDE.

#include <ArduinoBLE.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <mocopy.h>

using namespace mocopy;

// --------------------------- BLE defines -------------------------

#define JOINT_SERVICE_UUID "a9a95e92-26ea-4282-bd0c-7c8bd6c65a2b"
#define EXTERNAL_SERVICE_UUID "56176a63-d563-43f4-b239-636f41b63c6d"

// Characteristic UUIDs
// local uuids
#define PITCH_DIFF_CHARACTERISTIC_UUID "3ffdaee3-9acf-42ad-abe5-b078671f26da"
#define REP_COMPLETION_CHARACTERISTIC_UUID "08d54caf-75bc-4aa6-876b-8eea5427605a"
//#define KEY_FRAME_CHARACTERISTIC_UUID "5f9652ee-feef-42ef-b296-281b01a9a450"

// external uuids
#define JOINT_ORIENTATION_CHARACTERISTIC_UUID "b99cc0f3-8cdc-4bb1-a51d-3927431f0985"
#define EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID "04308b2c-90dc-4984-8c45-81650dff60b8"
#define RESET_BNO_JOINT_CHARACTERISTIC_UUID "356e9144-fd4f-4ad7-ad60-983f551e5c0c"
#define RESET_BNO_EXTERNAL_CHARACTERISTIC_UUID "c162bd0b-e48d-42c2-86f6-45ef8f615929"

// --------------------------- BLE defines -------------------------

// --------------------------- Hardware defines --------------------

// Motors for joint device
#define UP_MOTOR        D9
#define DOWN_MOTOR      D8
#define LEFT_MOTOR      D7
#define RIGHT_MOTOR     D6

// ----- Global Variables -----
Adafruit_BNO055 bno;
imu::Vector<3> joint_vector, external_vector, error_vector, correct_vector, calibrate_vector;
bool reset_bno_joint;
float joint_pitch, external_pitch;
char print_string[64];
// buffer for sending and recieving float data over BLE
byte buf[8] = {0};

// BLE variables
BLEDevice central, external;
BLEService joint_service(JOINT_SERVICE_UUID);
BLEService external_service(EXTERNAL_SERVICE_UUID);
BLEFloatCharacteristic rep_completion_characteristic(REP_COMPLETION_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEFloatCharacteristic pitch_diff_characteristic(PITCH_DIFF_CHARACTERISTIC_UUID, BLERead);
BLEBoolCharacteristic reset_bno_joint_characteristic(RESET_BNO_JOINT_CHARACTERISTIC_UUID, BLERead | BLEWrite);
BLECharacteristic reset_bno_external_characteristic;
//BLEFloatCharacteristic key_frame_characteristic(KEY_FRAME_CHARACTERISTIC_UUID, BLERead);

// Hardware variables
/*
                      (
                     (  ) (
                      )    )
         |||||||     (  ( (
        ( O   O )        )
 ____oOO___(_)___OOo____(
(_______________________)
*/
uint8_t vibes[3] = {0, 0, 0};

// ----- Global Variables -----
// initialize the BLE
void initBLE() {
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");

  // Construct the service to be advertised.
  joint_service.addCharacteristic(rep_completion_characteristic);
  joint_service.addCharacteristic(pitch_diff_characteristic);
  joint_service.addCharacteristic(reset_bno_joint_characteristic);
  //joint_service.addCharacteristic(key_frame_characteristic);
  BLE.addService(joint_service);

  // Setup external advertising.
  BLE.setLocalName("MoCopy (joint)");
  BLE.setAdvertisedService(joint_service);
  BLE.advertise();

  Serial.print("Advertising with address: ");
  Serial.println(BLE.address().c_str());

  // Scan for external services.
  Serial.println("Scanning for External service");
  do {
    BLE.scanForUuid(EXTERNAL_SERVICE_UUID);
    external = BLE.available();
  } while (!external);

  Serial.println("external device found.");
  Serial.print("Device MAC address: ");
  Serial.println(external.address());
  Serial.print("Device Name: ");
  Serial.println(external.localName());

  // Print all services advertised by the external.
  if (external.hasAdvertisedServiceUuid()) {
    Serial.println("External's advertised services UUIDs:");
    for (int i = 0; i < external.advertisedServiceUuidCount(); i++) {
      Serial.print(external.advertisedServiceUuid(i));
      Serial.print(", ");
    }
    Serial.println();
  } else {
    Serial.println("External has no advertised services!");
  }

  BLE.stopScan();
}

void updateBLE() {
  central = BLE.central();

  if (central) {
    Serial.print("Connected to central MAC: ");
    Serial.println(central.address());

    // Maybe change this to a for loop to make X attempts before quitting.
    if (!external.connected()) {
      if (external.connect()) {
        Serial.println("Successfully connected to External.");
      } else {
        Serial.println("Failed to connect to External.");
        Serial.print("Device MAC address: ");
        Serial.println(external.address());
        Serial.print("Device Name: ");
        Serial.println(external.localName());
        return;
      }
    }

    if (external.discoverAttributes()) {
      Serial.println("Successfully discovered External attributes.");
    } else {
      Serial.println("Failed to discover External attributes.");
      external.disconnect();
      return;
    }
    BLECharacteristic reset_bno_external_characteristic = external.characteristic(RESET_BNO_EXTERNAL_CHARACTERISTIC_UUID);
    BLECharacteristic joint_orientation = external.characteristic(JOINT_ORIENTATION_CHARACTERISTIC_UUID);
    BLECharacteristic external_orientation = external.characteristic(EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID);

    if (!joint_orientation || !external_orientation) {
      Serial.println("External device does not have the expected characteristic(s).");
      external.disconnect();
      return;
    } else if (!external_orientation.canSubscribe()) { // joint_orientation is BLEWrite threfore can't subscribe.
      Serial.println("Cannot subscribe to the External device's characteristic(s).");
      external.disconnect();
      return;
    }
    
    while (central.connected() && external.connected()) {
      if (reset_bno_joint_characteristic.written()){
        reset_bno_joint_characteristic.readValue(&reset_bno_joint, 1);
        if (reset_bno_joint){
          buf[0] = true;
          reset_bno_external_characteristic.writeValue(buf[0], 1);
          calibrate_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
        }
      }
      updateHardware();
      memcpy(buf, &joint_pitch, 4);
      joint_orientation.setValue(buf, 4);
      external_orientation.readValue(buf, 4);
      memcpy(&external_pitch, buf, 4);
      float diff = fabs(joint_pitch - external_pitch);
      pitch_diff_characteristic.setValue(diff);

      if (diff < 2) {
        rep_completion_characteristic.setValue(1);
      } else {
        rep_completion_characteristic.setValue(0);
      }
    }

    if (!central.connected()) {
      Serial.print("Disconnected from central MAC: ");
      Serial.println(central.address());
    }

    if (!external.connected()) {
      Serial.print("Disconnected from External MAC: ");
      Serial.println(external.address());
    }
  }
}

void updateHardware() {
  joint_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  joint_vector = joint_vector - calibrate_vector;
  joint_pitch = joint_vector[2];
  error_vector = joint_vector - correct_vector;

  // Calculate the vibration strength
  vibes[0] = max(0.0, min((uint8_t)(fabs(error_vector[0])) - GRACE_ANGLE_DEGREES + BASE_LED, MAX_LED));
  vibes[2] = max(0.0, min((uint8_t)(2 * fabs(error_vector[2])) - GRACE_ANGLE_DEGREES + BASE_LED, MAX_LED));

  if (error_vector[0] >= GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Right, ");
    analogWrite(RIGHT_MOTOR, vibes[0]);
    analogWrite(LEFT_MOTOR, 0);
  } else if (error_vector[0] <= -GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Left, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, vibes[0]);
  } else {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Grace, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, 0);
  }

  if (error_vector[2] >= GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Down, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, vibes[2]);
  } else if (error_vector[2] <= -GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Up, ");
    analogWrite(UP_MOTOR, vibes[2]);
    analogWrite(DOWN_MOTOR, 0);
  } else {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Grace, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, 0);
  } 

  delay(SAMPLE_PERIOD_MS);
}

void setup() {
  correct_vector = {0, 0, 0};
  initSerial();
  initHardware(bno, UP_MOTOR, DOWN_MOTOR, LEFT_MOTOR, RIGHT_MOTOR);
  reset_bno_joint = false;
  calibrateBNO(bno, calibrate_vector);
  initBLE();
}

void loop() {
  updateBLE();
}
