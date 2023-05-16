// BLE central device that creates a server to connect to the phone and
// Scans for a accelerometer service

#include <ArduinoBLE.h>
#include <mocopy.h>

using namespace mocopy;

// --------------------------- BLE defines -------------------------

// PIN Defines
#define BNO_RESET_PIN D2

// BLE Global Variables
BLEDevice joint, app;
BLEService central_service(CENTRAL_SERVICE_UUID);
BLEFloatCharacteristic key_frame_data_characteristic(KEY_FRAME_DATA_UUID, BLEWrite);
BLEBoolCharacteristic key_frame_hit_characteristic(KEY_FRAME_HIT_UUID, BLENotify);
BLEByteCharacteristic control_bits_characteristic(CONTROL_BITS_UUID, BLEWrite);
BLECharacteristic pitch_diff_characteristic; // from joint device
BLECharacteristic reset_bno_joint_characteristic; // from joint device

typedef enum state_t {
  idle_s,
  calibrate_s,
  pre_exercise_s,
  exercise_s,
  response_s
};
state_t state;

// Control bit definitions.
// which bit in the control_bits byte controls what :)
// #define CTRL_CAL_START 0
// #define CTRL_EXER_DONE 1
#define CTRL_CAL_START 48
#define CTRL_EXER_DONE 49
byte control_bits = 0b00000000;

unsigned long calibration_time, key_time;
float correct_pitch_diff, pitch_diff;

// buffer for reading in from (peripheral) untyped characteristics
byte buf[ANGLE_SIZE_BYTES] = {0};

// -------- Debug defines ---------

#define DEBUG_PRINTS 1

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
// TLDR: This function performs state transitions and side effects associated
// with those state transitions but NOTHING MORE.
void updateState() {
  switch (state) {
    case idle_s : {
      control_bits_characteristic.readValue(&control_bits, 1);
      if (control_bits == CTRL_CAL_START) {
        Serial.println("Calibrating...");
        control_bits = 0;
        control_bits_characteristic.writeValue(control_bits);
        calibration_time = millis();
        state = calibrate_s;
      }
    }
    break;
    case calibrate_s : {
      if (millis() - calibration_time >= CALIBRATION_TIME_MS) {
        Serial.println("Sending calibration data");
        // SEND RESET SIGNAL TO BNOs TO SET THEM TO <0, 0, 0>!
        buf[0] = true;
        reset_bno_joint_characteristic.writeValue(buf[0], 1);
        // digitalWrite(BNO_RESET_PIN, LOW);
        // delayMicroseconds(1);
        // digitalWrite(BNO_RESET_PIN, HIGH);
        // delay(1000); // delay for at least 800ms for BNO to restart.
        state = pre_exercise_s;
      }
    }
    break;
    case pre_exercise_s : {
      // wait to receive the first key frame.
      if (key_frame_data_characteristic.written()) {
        key_frame_data_characteristic.readValue(&correct_pitch_diff, ANGLE_SIZE_BYTES);
        key_time = millis();
        state = exercise_s;
      }
    }
    break;
    case exercise_s : {
      pitch_diff_characteristic.readValue(&pitch_diff, ANGLE_SIZE_BYTES);
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
      // wait to receive a new key frame.
      control_bits_characteristic.readValue(&control_bits, 1);
      if (control_bits == CTRL_EXER_DONE) {
        control_bits = 0;
        control_bits_characteristic.writeValue(control_bits);
        state = idle_s;
      } else if (key_frame_data_characteristic.written()) {
        key_frame_data_characteristic.readValue(&correct_pitch_diff, ANGLE_SIZE_BYTES);
        key_time = millis();
        state = exercise_s;
      }
    }
    break;
    default : break;
  }
}

bool keyFrameHit() {
  return fabs(pitch_diff - correct_pitch_diff) <= GRACE_ANGLE_DEGREES;
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

    pitch_diff_characteristic = joint.characteristic(PITCH_DIFF_CHARACTERISTIC_UUID);
    reset_bno_joint_characteristic = joint.characteristic(RESET_BNO_JOINT_CHARACTERISTIC_UUID);

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
  Serial.begin(9600);
  // while (!Serial); // only use this line if the serial monitor is needed
  delay(2000);

  pinMode(BNO_RESET_PIN, OUTPUT);
  digitalWrite(BNO_RESET_PIN, HIGH); // Reset is active low.

  Serial.println("Starting Mcpy central device...");

  initBLE();
}

void loop() {
  updateBLE();
}
