// An external device for the MoCopy® system, i.e. the outer most band
// on a particular limb such as the wrist on the arm.
// Install the ArduinoBLE library from the library manager in the IDE.

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <ArduinoBLE.h>
#include <mocopy.h>

using namespace mocopy;

// Motors for external device
#define UP_MOTOR        D7
#define DOWN_MOTOR      D9
#define LEFT_MOTOR      D6
#define RIGHT_MOTOR     D8

// BLE global varaibles
BLEDevice           joint;
BLEService          external_service        (EXTERNAL_SERVICE_UUID);
BLECharacteristic   snapshot_char           (E_SNAPSHOT_UUID,       BLEWrite,             DEFAULT_SIZE);
BLECharacteristic   key_frame_data_char     (E_KEY_FRAME_DATA_UUID, BLEWrite,             KEY_FRAME_SIZE);
BLECharacteristic   orientation_char        (E_ORIENTATION_UUID,    BLERead,              ORIENTATION_SIZE);
BLECharacteristic   calibrated_char         (E_CALIBRATED_UUID,     BLERead,              DEFAULT_SIZE);

Adafruit_BNO055 bno;
imu::Vector<3> kf_joint_vec, kf_diff_vec, kf_external_vec;
imu::Vector<3> external_vec, error_vec, snap_vec;
bool external_calibrated;

char printString [64];

byte buf[KEY_FRAME_SIZE] = {0};

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

void initBLE() {
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");

  // Construct the service to be advertised.
  external_service.addCharacteristic(snapshot_char);
  external_service.addCharacteristic(key_frame_data_char);
  external_service.addCharacteristic(orientation_char);
  external_service.addCharacteristic(calibrated_char);
  BLE.addService(external_service);

  // Setup peripheral advertising.
  BLE.setLocalName("MoCopy External");
  BLE.setAdvertisedService(external_service);
  BLE.advertise();
  Serial.print("Advertising with address: ");
  Serial.println(BLE.address().c_str());
}

void updateVectors() {
  key_frame_data_char.readValue(buf, KEY_FRAME_SIZE);
  parseKeyFrame(buf, kf_joint_vec, kf_diff_vec);

  kf_external_vec = kf_diff_vec + kf_joint_vec;

  external_vec = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  external_vec = external_vec - snap_vec;
  error_vec = external_vec - kf_external_vec;

  if (DEBUG_PRINT_VECTORS) {
    char vector_str[128];
    sprintf(vector_str, "external_vector: <%f, %f, %f>", 
      external_vec[0],
      external_vec[1],
      external_vec[2]
    );

    Serial.println(vector_str);
  }
}

void updateHardware() {
   // Calculate the vibration strength
  vibes[0] = max(0.0, min((uint8_t)(fabs(error_vec[0])) - GRACE_ANGLE + BASE_LED, MAX_LED));
  vibes[2] = max(0.0, min((uint8_t)(2 * fabs(error_vec[2])) - GRACE_ANGLE + BASE_LED, MAX_LED));

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

void updateBLE() {
  joint = BLE.central();

  if (joint) {
    Serial.print("Connected to joint MAC: ");
    Serial.println(joint.address());

    while (joint.connected()) {
      if (!external_calibrated) {
        external_calibrated = getBNOCalibration(bno);
        buf[0] = 1;
        calibrated_char.writeValue(buf, DEFAULT_SIZE);
      }

      if (snapshot_char.written()) {
        snapshot_char.readValue(buf, DEFAULT_SIZE);
        // if snapshot is true
        if (buf[0]) {
          snap_vec = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
          Serial.println("Calibrated.");
        }
      }

      updateVectors();
      updateHardware();

      delay(SAMPLE_PERIOD_MS);
    }

    Serial.println("Disconnected from joint MAC: ");
    Serial.println(joint.address());
  }
}

void setup() {
  initSerial();
  initHardware(bno, UP_MOTOR, DOWN_MOTOR, LEFT_MOTOR, RIGHT_MOTOR);
  initBLE();
}

void loop() {
  updateBLE();
}
