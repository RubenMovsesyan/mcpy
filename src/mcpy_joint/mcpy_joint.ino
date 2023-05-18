// MoCopy joint device (between external and central).
// Install the ArduinoBLE library from the library manager in the IDE.

#include <ArduinoBLE.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <mocopy.h>

using namespace mocopy;

// --------------------------- Hardware defines --------------------

// Motors for joint device
#define UP_MOTOR        D6
#define DOWN_MOTOR      D7
#define LEFT_MOTOR      D9
#define RIGHT_MOTOR     D8

// ----- Global Variables -----
Adafruit_BNO055 bno;
imu::Vector<3> kf_joint_vec, kf_diff_vec;
imu::Vector<3> external_vec, joint_vec, error_vec, snap_vec, diff_vec;
bool external_calibrated = false, joint_calibrated = false;
bool both_calibrated_fuse = false;
//   ^ used to prevent constants writes to the both_calibrated characteristic.

char print_string[64];
// buffer for sending and recieving float data over BLE
byte buf[KEY_FRAME_SIZE] = {0};

// BLE global varaibles
BLEDevice           central, external;
BLEService          joint_service           (JOINT_SERVICE_UUID);
BLECharacteristic   snapshot_char           (J_SNAPSHOT_UUID,       BLEWrite,             DEFAULT_SIZE);
BLECharacteristic   key_frame_data_char     (J_KEY_FRAME_DATA_UUID, BLEWrite,             KEY_FRAME_SIZE);
BLECharacteristic   key_frame_hit_char      (J_KEY_FRAME_HIT_UUID,  BLERead | BLEWrite,   DEFAULT_SIZE);
BLECharacteristic   both_calibrated_char    (BOTH_CALIBRATED_UUID,  BLERead,              DEFAULT_SIZE);

// BLE external characteristics
BLECharacteristic   e_snapshot_char;
BLECharacteristic   e_key_frame_data_char;
BLECharacteristic   e_orientation_char;
BLECharacteristic   e_calibrated_char;

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
  joint_service.addCharacteristic(snapshot_char);
  joint_service.addCharacteristic(key_frame_data_char);
  joint_service.addCharacteristic(key_frame_hit_char);
  joint_service.addCharacteristic(both_calibrated_char);
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

    // connecting to external device
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
    
    e_snapshot_char = external.characteristic(E_SNAPSHOT_UUID);
    e_key_frame_data_char = external.characteristic(E_KEY_FRAME_DATA_UUID);
    e_orientation_char = external.characteristic(E_ORIENTATION_UUID);
    e_calibrated_char = external.characteristic(E_CALIBRATED_UUID);
    
    while (central.connected() && external.connected()) {
      if (!joint_calibrated) {
        // only set joint calibration once.
        joint_calibrated = getBNOCalibration(bno);
      }

      // Testing calibration of both devices
      if (!external_calibrated) {
        // only read external calibration once.
        e_calibrated_char.readValue(&external_calibrated, DEFAULT_SIZE);
      }

      if (external_calibrated && joint_calibrated && !both_calibrated_fuse) {
        // both_calibrated_fuse is used to ensure this if statement is only ever
        // entered once to prevent constant writes which might slow the device.
        both_calibrated_char.writeValue(&joint_calibrated, DEFAULT_SIZE);
        both_calibrated_fuse = true; // trip the fuse, never enter this if again.
      }

      if (snapshot_char.written()){
        snapshot_char.readValue(buf, DEFAULT_SIZE);
        // if snapshot is true
        if (buf[0]) {
          e_snapshot_char.writeValue(buf, DEFAULT_SIZE);
          snap_vec = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
          Serial.println("Calibrated.");
        }
      }

      updateVectors();
      updateHardware();

      // Considering commenting this out to improve performance?
      delay(SAMPLE_PERIOD_MS);
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

void updateVectors() {
  // reading external orientation data
  e_orientation_char.readValue(buf, ORIENTATION_SIZE);
  parseOrientation(buf, external_vec);

  if (key_frame_data_char.written()) {
    key_frame_data_char.readValue(buf, KEY_FRAME_SIZE);
    e_key_frame_data_char.writeValue(buf, KEY_FRAME_SIZE);
    parseKeyFrame(buf, kf_joint_vec, kf_diff_vec);
  }

  joint_vec = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  joint_vec = joint_vec - snap_vec;
  error_vec = joint_vec - kf_joint_vec;
  diff_vec = external_vec - joint_vec;

  if (keyFrameHit()) {
    // buf[0] = KF_SUCCESS;
    // key_frame_hit_char.writeValue(buf[0], DEFAULT_SIZE);
  }

  if (DEBUG_PRINT_VECTORS) {
    Serial.print("Diff: ");
    printVector(print_string, diff_vec, false);
    Serial.print(" Joint: ");
    printVector(print_string, joint_vec, true);
  }
}

void updateHardware() {
  // Calculate the vibration strength
  vibes[0] = max(0.0, min((uint8_t)(fabs(error_vec[0])) - GRACE_ANGLE + BASE_LED, MAX_LED));
  // Removed 2 * fabs(error_vec[2]).
  vibes[2] = max(0.0, min((uint8_t)(fabs(error_vec[2])) - GRACE_ANGLE + BASE_LED, MAX_LED));

  if (error_vec[0] >= GRACE_ANGLE) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Right, ");
    analogWrite(RIGHT_MOTOR, vibes[0]);
    analogWrite(LEFT_MOTOR, 0);
  } else if (error_vec[0] <= -GRACE_ANGLE) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Left, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, vibes[0]);
  } else {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Grace, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, 0);
  }

  if (error_vec[2] >= GRACE_ANGLE) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Down, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, vibes[2]);
  } else if (error_vec[2] <= -GRACE_ANGLE) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Up, ");
    analogWrite(UP_MOTOR, vibes[2]);
    analogWrite(DOWN_MOTOR, 0);
  } else {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Grace, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, 0);
  }
}

bool keyFrameHit() {
  // Note that diff is also compared to +/- GRACE_ANGLE.
  return (
    (fabs(joint_vec[0] - kf_joint_vec[0]) <= GRACE_ANGLE) &&
    // fabs((joint_vec[1] - kf_joint_vec[1]) <= GRACE_ANGLE) && // don't care about roll
    (fabs(joint_vec[2] - kf_joint_vec[2]) <= GRACE_ANGLE) &&
    (fabs(diff_vec[0] - kf_diff_vec[0]) <= GRACE_ANGLE) &&
    // fabs((diff_vec[0] - kf_diff_vec[0]) <= GRACE_ANGLE) && // don't care about roll
    (fabs(diff_vec[2] - kf_diff_vec[2]) <= GRACE_ANGLE)
  );
}

void setup() {
  initSerial();
  initHardware(bno, UP_MOTOR, DOWN_MOTOR, LEFT_MOTOR, RIGHT_MOTOR);
  initBLE();
}

void loop() {
  updateBLE();
}
