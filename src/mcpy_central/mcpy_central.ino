// BLE central device that creates a server to connect to the phone and
// Scans for a accelerometer service

#include <ArduinoBLE.h>
#include <mocopy.h>

using namespace mocopy;

// BLE Global Variables
BLEDevice           joint, app;
BLEService          central_service         (CENTRAL_SERVICE_UUID);
BLECharacteristic   key_frame_data_char     (C_KEY_FRAME_DATA_UUID,   BLEWrite,             KEY_FRAME_SIZE);
BLECharacteristic   key_frame_hit_char      (C_KEY_FRAME_HIT_UUID,    BLERead | BLEWrite,   DEFAULT_SIZE);
BLECharacteristic   control_char            (CONTROL_UUID,            BLERead | BLEWrite,   DEFAULT_SIZE);

// BLE joint characteristics
BLECharacteristic   j_key_frame_data_char;
BLECharacteristic   j_key_frame_hit_char;
BLECharacteristic   j_snapshot_char;
BLECharacteristic   both_calibrated_char;

// State machine
typedef enum State {
  IDLE,
  CALIBRATE,
  SNAPSHOT,
  PRE_EXERCISE,
  EXERCISE,
  RESPONSE
};
State state;

// ===== Global varaibles =====
uint8_t control = 0;
bool both_calibrated;
unsigned long kf_time;

// buffer for reading in data from untyped characteristics
byte buf[KEY_FRAME_SIZE] = {0};

// This initializes the BLE server for the phone to connect to
void initBLE() {
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  Serial.println("Successfully initialized Bluetooth® Low Energy module.");

  // Construct the service to be advertised
  central_service.addCharacteristic(key_frame_data_char);
  central_service.addCharacteristic(key_frame_hit_char);
  central_service.addCharacteristic(control_char);
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
    case IDLE : {
      control_char.readValue(&control, DEFAULT_SIZE);
      if (control == CTRL_CAL_START) {
        Serial.println("Calibrating...");
        state = CALIBRATE;
      }
    }
    break;
    case CALIBRATE : {
      both_calibrated_char.readValue(&both_calibrated, DEFAULT_SIZE);
      if (both_calibrated) { 
        control = CTRL_CAL_DONE;
        control_char.writeValue(control);
        state = SNAPSHOT;
      }
    }
    break;
    case SNAPSHOT : {
      control_char.readValue(&control, DEFAULT_SIZE);
      if (control == CTRL_TAKE_SNAP) {
        Serial.println("Sending calibration data");
        buf[0] = true;
        // Forward the reset signal to other devices.
        j_snapshot_char.writeValue(buf[0], DEFAULT_SIZE);
        state = PRE_EXERCISE;
      }
    }
    break;
    case PRE_EXERCISE : {
      // wait to receive the first key frame.
      if (key_frame_data_char.written()) {
        key_frame_data_char.readValue(buf, KEY_FRAME_SIZE);
        // forward key frame data to joint
        j_key_frame_data_char.writeValue(buf, KEY_FRAME_SIZE);
        kf_time = millis();
        state = EXERCISE;
      }
    }
    break;
    case EXERCISE : {
      j_key_frame_hit_char.readValue(buf, DEFAULT_SIZE);
      if (buf[0] == KF_SUCCESS) {
        key_frame_hit_char.writeValue(buf, DEFAULT_SIZE);
        state = RESPONSE;
      } else if (millis() - kf_time >= KEY_TIMEOUT_MS) {
        buf[0] = KF_MISS;
        key_frame_hit_char.writeValue(buf, DEFAULT_SIZE);
        state = RESPONSE;
      }
    }
    break;
    case RESPONSE : {
      // wait to receive a new key frame or exercise_done.
      control_char.readValue(&control, DEFAULT_SIZE);
      if (control == CTRL_EXER_DONE) {
        state = IDLE;
      } else if (key_frame_data_char.written()) {
        key_frame_data_char.readValue(buf, KEY_FRAME_SIZE);
        j_key_frame_data_char.writeValue(buf, KEY_FRAME_SIZE);
        kf_time = millis();
        state = EXERCISE;
      }
    }
    break;
    default : break;
  }
}

void updateBLE() {
  app = BLE.central();

  if (app) {
    Serial.print("Connected to app MAC: ");
    Serial.println(app.address());

    // New connection
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

    j_key_frame_data_char = joint.characteristic(J_KEY_FRAME_DATA_UUID);
    j_key_frame_hit_char = joint.characteristic(J_KEY_FRAME_HIT_UUID);
    j_snapshot_char = joint.characteristic(J_SNAPSHOT_UUID);
    both_calibrated_char = joint.characteristic(BOTH_CALIBRATED_UUID);

    while (app.connected() && joint.connected()) {
      updateState();
    }

    // Disconnected
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
