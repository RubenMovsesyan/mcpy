// BLE central device that creates a server to connect to the phone and
// Scans for a accelerometer service

#include <ArduinoBLE.h>
#include <Servo.h>

// --------------------------- BLE defines -------------------------

// Local UUIDs
#define CENTRAL_SERVICE_UUID "0c35e466-ad83-4651-88fa-0ff9d70fbf8c"
#define FORWARD_CHARACTERISTIC_UUID "a59d3afb-5010-43f0-a241-1ad27e92d7b9"
#define EXERCISE_INFO_CHARACTERISTIC_UUID "f75061a7-e391-4b61-ae4b-95812a2086e3"
#define KEY_FRAME_HIT_UUID "0180ef1a-ef68-11ed-a05b-0242ac120003"

// UUIDs for joint device
#define JOINT_SERVICE_UUID "a9a95e92-26ea-4282-bd0c-7c8bd6c65a2b"
#define REP_COMPLETION_CHARACTERISTIC_UUID "08d54caf-75bc-4aa6-876b-8eea5427605a"
#define PITCH_DIFF_CHARACTERISTIC_UUID "3ffdaee3-9acf-42ad-abe5-b078671f26da"


// --------------------------- BLE defines -------------------------


// ------ State Machine defines ------

#define IDLE                  0
#define CALIBRATION           1
#define PRE_EXERCISE          2
#define EXERCISE              3
#define RESPONSE              4

// ------ State Machine defines ------

#define STR_SIZE              64
#define EXER_INFO_SIZE        512
#define PITCH_THRESHOLD       2

char print_string[STR_SIZE];

BLEDevice joint, app;
BLEService central_service(CENTRAL_SERVICE_UUID);
BLEFloatCharacteristic forward_characteristic(FORWARD_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLECharacteristic exercise_info_characteristic(EXERCISE_INFO_CHARACTERISTIC_UUID, BLERead | BLEWrite | BLENotify, EXER_INFO_SIZE);
BLECharacteristic key_frame_hit_characteristic(KEY_FRAME_HIT_UUID, BLEWrite | BLENotify);

byte buf[4] = {0};
float diff = 0.0;
byte exer_info_buf[EXER_INFO_SIZE] = {0};
// int calibration_timer;

int state_machine;

// -------- Exercise info variables ---------------

int num_reps;
int num_keyframes;
float keyframes[EXER_INFO_SIZE - 2] = {0};
int curr_keyframe, key_frame_index;
bool calibrated;

// -------- Exercise info variables ---------------

// -------- Debug defines ---------

#define DEBUG_PRINTS 1

// -------- Debug defines ---------

// temp variables
int light_counter;
#define LED D7


// This initializes the BLE server for the phone to connect to
void initBLE() {
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  Serial.println("Successfully initialized Bluetooth® Low Energy module.");

  // Construct the service to be advertised
  central_service.addCharacteristic(forward_characteristic);
  central_service.addCharacteristic(exercise_info_characteristic);
  central_service.addCharacteristic(key_frame_hit_characteristic);
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

void updateStateMachine(BLECharacteristic pitch_diff_characteristic) {
  switch(state_machine) {
    case IDLE: {
      if (DEBUG_PRINTS) Serial.println("&& State: IDLE");

      if (exercise_info_characteristic.written()) {
        exercise_info_characteristic.readValue(exer_info_buf, EXER_INFO_SIZE);
        // distribute exercise info into variables
        num_reps = exer_info_buf[0];
        num_keyframes = exer_info_buf[1];
        // memcpy(keyframes, exer_info_buf, EXER_INFO_SIZE - 2);
        // for (int i = 0; i < (EXER_INFO_SIZE - 2) / 4; i += 4) {
        //   keyframes[i] = exer_info_buf[(i / 4) + 2];
        // }
        calibrated = false;
        state_machine = CALIBRATION;
      }
    }
      break;
    case CALIBRATION: {
      if (DEBUG_PRINTS) Serial.println("&& State: CALIBRATION");

      Serial.println(num_reps);
      Serial.println(num_keyframes);

      for (int i = 0; i < EXER_INFO_SIZE; i++) {
        Serial.print(exer_info_buf[i]);
        Serial.print(" | ");
      }

      Serial.println();
      //unsigned long previousTime = millis();
      delay(5000);
      // "calibrate"
      calibrated = true;
      curr_keyframe = 0;
      key_frame_index = 0;
      // send calibrated to app here
      state_machine = PRE_EXERCISE;
    }
      break;
    case PRE_EXERCISE: {
      if (DEBUG_PRINTS) Serial.println("&& State: PRE_EXERCISE");
      // set the keyframe to 0 here
      if (curr_keyframe >= num_reps * num_keyframes) {
        state_machine = IDLE;
      } else {
        state_machine = EXERCISE;
      }
    }
      break;
    case EXERCISE: {
      if (DEBUG_PRINTS) Serial.println("&& State: EXERCISE");
      // rep_completion_characteristic.readValue(buf, 4);
      // memcpy(&rep_completion, buf, 4);
      // forward_characteristic.setValue(rep_completion);
      float pitch_diff = 0;
      
      float actual_pitch_diff = keyframes[key_frame_index + 2];
      pitch_diff_characteristic.readValue(buf, 4);
      memcpy(&pitch_diff, buf, 4);

      if (curr_keyframe < num_reps * num_keyframes) {
        Serial.println(fabs(pitch_diff - actual_pitch_diff));
        if (fabs(pitch_diff - actual_pitch_diff) <= PITCH_THRESHOLD) {
          // key frame count ++
          light_counter = 1000;
          curr_keyframe++;

          key_frame_index++;
          if (key_frame_index >= num_keyframes) {
            key_frame_index = 0;
          }
          // set key frame data
          state_machine = RESPONSE;
        } 
        // else if (/* timeout */) {
        //   // set timeout data
        //   state_machine = RESPONSE;
        // }
      } else if (curr_keyframe >= num_reps * num_keyframes) {
        state_machine = PRE_EXERCISE;
      } else {
        state_machine = RESPONSE;
      }
    }
      break;
    case RESPONSE: {
      if (DEBUG_PRINTS) Serial.println("&& State: RESPONSE");
      // send timeout / key frame data
      Serial.print("Current keyframe: ");
      Serial.println(key_frame_index);
      state_machine = EXERCISE;
    }
      break;
    default:
      if (DEBUG_PRINTS) Serial.println("&& State: DEFAULT");
      break;
  }
}

void tempTurnLightOn() {
  if (light_counter) {
    digitalWrite(LED, HIGH);
    light_counter--;
  }
}

void updateBLE() {
  app = BLE.central();

  if (app) {
    Serial.print("Connected to app MAC: ");
    Serial.println(app.address());

    if (!joint.connected()) {
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
    }

    if (joint.discoverAttributes()) {
      Serial.println("Successfully discovered Joint attributes.");
    } else {
      Serial.println("Failed to discover Joint attributes.");
      joint.disconnect();
      return;
    }

    BLECharacteristic rep_completion_characteristic = joint.characteristic(REP_COMPLETION_CHARACTERISTIC_UUID);
    BLECharacteristic pitch_diff_characteristic = joint.characteristic(PITCH_DIFF_CHARACTERISTIC_UUID);

    if (!rep_completion_characteristic) {
      Serial.println("Joint device does not have the expected characteristic(s).");
      joint.disconnect();
      return;
    } else if (!rep_completion_characteristic.canSubscribe()) {
      Serial.println("Cannot subscribe to the Joint device's characteristic(s).");
      joint.disconnect();
      return;
    }

    while (app.connected() && joint.connected()) {
      updateStateMachine(pitch_diff_characteristic);
      tempTurnLightOn();
    }

    if (!app.connected()) {
      Serial.print("Disconnected from app MAC: ");
      Serial.println(app.address());
    }

    if (joint.connected()) {
      Serial.print("Disconnected from Joint MAC: ");
      Serial.println(joint.address());
    }
  }
}

// Arduino Functions \/ ------------------------------------------ \/
 
void setup() {
  Serial.begin(9600);
  // while (!Serial); // only use this line if the serial monitor is needed
  delay(2000);
  Serial.println("Starting Mcpy central device...");

  state_machine = IDLE;
  calibrated = false;

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // starting keyframes with dummy data for testing
  keyframes[0] = 0;
  keyframes[1] = 20;
  keyframes[2] = 45;
  keyframes[3] = 90;

  initBLE();
}

void loop() {
  updateBLE();
}
