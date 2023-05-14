// An external device for the MoCopy® system, i.e. the outer most band
// on a particular limb such as the wrist on the arm.
// Install the ArduinoBLE library from the library manager in the IDE.

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <ArduinoBLE.h>

#define DEBUG_PRINT_BLE 1
#define DEBUG_PRINT_STATUS 0

#define EXTERNAL_SERVICE_UUID "56176a63-d563-43f4-b239-636f41b63c6d"
#define JOINT_ORITENTATION_CHARACTERISTIC_UUID "b99cc0f3-8cdc-4bb1-a51d-3927431f0985"
#define EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID "04308b2c-90dc-4984-8c45-81650dff60b8"
#define RESET_BNO_EXTERNAL_CHARACTERISTIC_UUID "c162bd0b-e48d-42c2-86f6-45ef8f615929"

// --------------------------- Hardware defines --------------------

#define BNO_RESET       D12

// Motors
#define UP_MOTOR        D9
#define DOWN_MOTOR      D7
#define LEFT_MOTOR      D6
#define RIGHT_MOTOR     D8

// --------------------------- Hardware defines --------------------

// ------ Math defines --------

#define SAMPLE_PERIOD_MS 100
#define YAW_GRACE_ANGLE_DEGREES 15
#define PITCH_GRACE_ANGLE_DEGREES 15
#define MAX_VIBRATION 255
#define BASE_VIBRATION 120 // determined experimentally
#define SAMPLE_PERIOD_MS 10

// ------ Math defines --------

BLEService externalService(EXTERNAL_SERVICE_UUID);
BLEFloatCharacteristic joint_orientation(JOINT_ORITENTATION_CHARACTERISTIC_UUID, BLEWrite);
BLEFloatCharacteristic external_orientation(EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEBoolCharacteristic reset_BNO(RESET_BNO_EXTERNAL_CHARACTERISTIC_UUID, BLEWrite);
BLEDevice joint;

Adafruit_BNO055 bno;
imu::Vector<3> joint_euler_vector, external_euler_vector, correct_vector, error_vector, starting_vector;
float joint_pitch, external_pitch;
bool bno_reset;

char printString [64];
byte buf[4] = {0};

uint8_t vibes[3] = {0, 0, 0};

// initialize the BNO055 and all the pins
void initHardware() {
  // start the bno
  if (!bno.begin(OPERATION_MODE_NDOF)) {
    Serial.println("\nFailed to find BNO055 chip");
    while (1);
  }

  Serial.println("\nBNO055 Found!");
  bno.enterNormalMode();

  starting_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  bno_reset = false;

  // init motor pins
  analogWriteResolution(8);
  pinMode(UP_MOTOR,     OUTPUT);
  pinMode(DOWN_MOTOR,   OUTPUT);
  pinMode(LEFT_MOTOR,   OUTPUT);
  pinMode(RIGHT_MOTOR,  OUTPUT);

  Serial.println("Mcpy hardware initialized");

  correct_vector = {0, 0, 0};
}

void initBLE() {
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");

  // Construct the service to be advertised.
  externalService.addCharacteristic(joint_orientation);
  externalService.addCharacteristic(external_orientation);
  externalService.addCharacteristic(reset_BNO);
  BLE.addService(externalService);

  // Setup peripheral advertising.
  BLE.setLocalName("MoCopy External");
  BLE.setAdvertisedService(externalService);
  BLE.advertise();
  Serial.print("Advertising with address: ");
  Serial.println(BLE.address().c_str());
}

void setup() {
  Serial.begin(9600);
  delay(2000); // wait for Serial
  Serial.println("Starting Mcpy External...");

  initHardware();
  initBLE();
}

void updateHardware() {
  external_euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  external_euler_vector = external_euler_vector - starting_vector;
  external_pitch = external_euler_vector[2];

  if (DEBUG_PRINT_BLE) {
    Serial.print("Joint pitch: ");
    Serial.print(joint_pitch);
    Serial.print(" External pitch: ");
    Serial.println(external_pitch);
  }

  error_vector = external_euler_vector - correct_vector;

  // Calculate the vibration strength
  vibes[0] = max(0.0, min((uint8_t)(fabs(error_vector[0])) - YAW_GRACE_ANGLE_DEGREES + BASE_VIBRATION, MAX_VIBRATION));
  vibes[2] = max(0.0, min((uint8_t)(2 * fabs(error_vector[2])) - PITCH_GRACE_ANGLE_DEGREES + BASE_VIBRATION, MAX_VIBRATION));

  if (error_vector[0] >= YAW_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Right, ");
    analogWrite(RIGHT_MOTOR, vibes[0]);
    analogWrite(LEFT_MOTOR, 0);
  } else if (error_vector[0] <= -YAW_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Left, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, vibes[0]);
  } else {
    if (DEBUG_PRINT_STATUS) Serial.print("Grace, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, 0);
  }

  if (error_vector[2] >= PITCH_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Down, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, vibes[2]);
  } else if (error_vector[2] <= -PITCH_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Up, ");
    analogWrite(UP_MOTOR, vibes[2]);
    analogWrite(DOWN_MOTOR, 0);
  } else {
    if (DEBUG_PRINT_STATUS) Serial.print("Grace, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, 0);
  } 

  delay(SAMPLE_PERIOD_MS);
}

void updateBLE() {
  joint = BLE.central();
  // Note: The peripheral does not attempt a connection to the joint and thus
  // does not call the connect() method.

  if (joint) {
    Serial.print("Connected to joint MAC: ");
    Serial.println(joint.address());

    while (joint.connected()) {
      updateHardware();
      joint_orientation.readValue(buf, 4);
      memcpy(&joint_pitch, buf, 4);
      external_orientation.setValue(external_pitch);

      if (reset_BNO.written()) {
        reset_BNO.readValue(&bno_reset, 1);
        if (bno_reset) {
          starting_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
        }
      }
    }

    Serial.println("Disconnected from joint MAC: ");
    Serial.println(joint.address());
  }
}

void loop() {
  updateBLE();
}
