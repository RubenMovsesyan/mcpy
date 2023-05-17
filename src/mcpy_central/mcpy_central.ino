// BLE central device that creates a server to connect to the phone and
// Scans for a accelerometer service

#include <ArduinoBLE.h>
#include <mocopy.h>

using namespace mocopy;

// BLE Global Variables
BLEDevice joint, app;
BLEService central_service(CENTRAL_SERVICE_UUID);
BLECharacteristic key_frame_data_characteristic(KEY_FRAME_DATA_UUID, BLEWrite, 24);
BLEBoolCharacteristic key_frame_hit_characteristic(KEY_FRAME_HIT_UUID, BLENotify | BLERead);
BLEByteCharacteristic control_bits_characteristic(CONTROL_BITS_UUID, BLERead | BLEWrite);
BLECharacteristic orientation_diff_characteristic; // from joint device
BLECharacteristic reset_bno_joint_characteristic; // from joint device
BLECharacteristic both_wiggles_characteristic; // from joint device
BLECharacteristic joint_key_frame_data_characteristic;
typedef enum state_t {
  idle_s,
  wiggle_s,
  calibrate_s,
  pre_exercise_s,
  exercise_s,
  response_s
};
state_t state;

// Control bit definitions.
#define CTRL_CAL_START 48
#define CTRL_CAL_DONE 49
#define CTRL_TAKE_SNAP 50
#define CTRL_EXER_DONE 51
byte control_bits = 0b00000000;

bool both_wiggles;
unsigned long key_time;
float joint_yaw, joint_roll, joint_pitch, correct_yaw_diff, correct_roll_diff, correct_pitch_diff, yaw_diff, roll_diff, pitch_diff;
//joint_euler and correct_euler_diff is from device to save to buf
//euler_diff is from joint

// buffer for reading in from (peripheral) untyped characteristics
byte buf[24] = {0};

// This initializes the BLE server for the phone to connect to
void initBLE() {
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  Serial.println("Successfully initialized Bluetooth® Low Energy module.");

  // Construct the service to be advertised
  central_service.addCharacteristic(key_frame_hit_characteristic);
  central_service.addCharacteristic(key_frame_data_characteristic);
  central_service.addCharacteristic(control_bits_characteristic);
  BLE.addService(central_service);

  // Setup central advertising
  BLE.setLocalName("MoCopy (central)");
  BLE.setAdvertisedService(central_service);
  BLE.advertise();

  Serial.print("Advertising with address: ");
  Serial.println(BLE.address().c_str());

  // Scan for joint service
  Serial.println("Scanning for Joint service");
  do {
    BLE.scanForUuid(JOINT_SERVICE_UUID);
    joint = BLE.available();
  } while (!joint);

  Serial.println("Joint device found.");
  Serial.print("Device MAC address: ");
  Serial.println(joint.address());
  Serial.print("Device Name: ");
  Serial.println(joint.localName());

  // Print all service advertised by the joint
  if (joint.hasAdvertisedServiceUuid()) {
    Serial.println("Joint's advertised services UUIDs: ");
    for (int i = 0; i < joint.advertisedServiceUuidCount(); i++) {
      Serial.print(joint.advertisedServiceUuid(i));
      Serial.print(", ");
    }
    Serial.println();
  } else {
    Serial.println("Joint has no advertised services!");
  }

  BLE.stopScan();
}

// To avoid an excessive number of states the state machine is Mealy meaning it
// performs changes *on state transitions* and not exclusively within states.
void updateState() {
  switch (state) {
    case idle_s : {
      control_bits_characteristic.readValue(&control_bits, 1);
      if (control_bits == CTRL_CAL_START) {
        Serial.println("Calibrating...");
        control_bits = 0;
        control_bits_characteristic.writeValue(control_bits);
        state = wiggle_s;
      }
    }
    break;
    case wiggle_s : {
      both_wiggles_characteristic.readValue(&both_wiggles, 1);
      if (both_wiggles) { 
        control_bits = CTRL_CAL_DONE;
        control_bits_characteristic.writeValue(control_bits);
        state = calibrate_s;
      }
    }
    break;
    case calibrate_s : {
      control_bits_characteristic.readValue(&control_bits, 1);
      if (control_bits == CTRL_TAKE_SNAP) {
        Serial.println("Sending calibration data");
        buf[0] = true;
        // Forward the reset signal to other devices.
        reset_bno_joint_characteristic.writeValue(buf[0], 1);
        state = pre_exercise_s;
      }
    }
    break;
    case pre_exercise_s : {
      // wait to receive the first key frame.
      if (key_frame_data_characteristic.written()) {
        key_frame_data_characteristic.readValue(buf, 24);
        joint_key_frame_data_characteristic.writeValue(buf, 24);
        memcpy(&joint_yaw, &buf[0], 4);
        memcpy(&joint_roll, &buf[4], 4);
        memcpy(&joint_pitch, &buf[8], 4);
        memcpy(&correct_yaw_diff, &buf[12], 4);
        memcpy(&correct_roll_diff, &buf[16], 4);
        memcpy(&correct_pitch_diff, &buf[20], 4);
        key_time = millis();
        state = exercise_s;
      }
    }
    break;
    case exercise_s : {
      orientation_diff_characteristic.readValue(buf, 12);
      memcpy(&yaw_diff, &buf[0], 4);
      memcpy(&roll_diff, &buf[4], 4);
      memcpy(&pitch_diff, &buf[8], 4);
      if (keyFrameHit()) {
        key_frame_hit_characteristic.writeValue(KF_SUCCESS);
        state = response_s;
      } else if (millis() - key_time >= KEY_TIMEOUT_MS) {
        key_frame_hit_characteristic.writeValue(KF_MISS);
        state = response_s;
      }
    }
    break;
    case response_s : {
      // wait to receive a new key frame or exercise_done.
      control_bits_characteristic.readValue(&control_bits, 1);
      if (control_bits == CTRL_EXER_DONE) {
        control_bits = 0;
        control_bits_characteristic.writeValue(control_bits);
        state = idle_s;
      } else if (key_frame_data_characteristic.written()) {
        key_frame_data_characteristic.readValue(buf, 24);
        joint_key_frame_data_characteristic.writeValue(buf, 24);
        memcpy(&joint_yaw, &buf[0], 4);
        memcpy(&joint_roll, &buf[4], 4);
        memcpy(&joint_pitch, &buf[8], 4);
        memcpy(&correct_yaw_diff, &buf[12], 4);
        memcpy(&correct_roll_diff, &buf[16], 4);
        memcpy(&correct_pitch_diff, &buf[20], 4);
        key_time = millis();
        state = exercise_s;
      }
    }
    break;
    default : break;
  }
}

bool keyFrameHit() {
  return ((fabs(pitch_diff - correct_pitch_diff) <= GRACE_ANGLE_DEGREES) && (fabs(yaw_diff - correct_yaw_diff) <= GRACE_ANGLE_DEGREES));
}

void updateBLE() {
  app = BLE.central();

  if (app) {
    Serial.print("Connected to app MAC: ");
    Serial.println(app.address());

    if (joint.connect()) {
      Serial.println("Successfully connected to Joint.");
    } else {
      Serial.println("Failed to connect to Joint.");
      Serial.print("Device MAC address: ");
      Serial.println(joint.address());
      Serial.print("Device Name: ");
      Serial.println(joint.localName());
      return;
    }

    if (joint.discoverAttributes()) {
      Serial.println("Successfully discovered Joint attributes.");
    } else {
      Serial.println("Failed to discover Joint attributes.");
      joint.disconnect();
      return;
    }

    orientation_diff_characteristic = joint.characteristic(ORIENTATION_DIFF_CHARACTERISTIC_UUID);
    reset_bno_joint_characteristic = joint.characteristic(RESET_BNO_JOINT_CHARACTERISTIC_UUID);
    both_wiggles_characteristic = joint.characteristic(BOTH_WIGGLES_CHARACTERISTIC_UUID);
    joint_key_frame_data_characteristic = joint.characteristic(JOINT_KEY_FRAME_DATA_CHARACTERISTIC_UUID);
    while (app.connected() && joint.connected()) {
      updateState();
    }

    if (!app.connected()) {
      Serial.print("Disconnected from app MAC: ");
      Serial.println(app.address());
    }

    if (!joint.connected()) {
      Serial.print("Disconnected from Joint MAC: ");
      Serial.println(joint.address());
    }
  }
}
 
void setup() {
  initSerial();
  Serial.println("Starting Mcpy central device...");

  initBLE();
}

void loop() {
  updateBLE();
}
