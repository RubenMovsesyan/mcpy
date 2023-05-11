// Example to communicate BLE characteristics between an
// Arduino and the smartphone app.

#include <ArduinoBLE.h>

#define CENTRAL_SERVICE_UUID "0c35e466-ad83-4651-88fa-0ff9d70fbf8c"
#define KEY_FRAME_DATA_UUID "b26dd24c-6bff-417c-aa16-c857b25b9c28"
#define KEY_FRAME_HIT_UUID "0180ef1a-ef68-11ed-a05b-0242ac120003"
#define CONTROL_BITS_UUID "a10fb559-3be8-40e2-aaca-27721b853a71"

BLEDevice app;
BLEService central_service(CENTRAL_SERVICE_UUID);
BLEDoubleCharacteristic key_frame_data_characteristic(KEY_FRAME_DATA_UUID, BLEWrite);
BLEBoolCharacteristic key_frame_hit_characteristic(KEY_FRAME_HIT_UUID, BLERead | BLENotify);
BLEByteCharacteristic control_bits_characteristic(CONTROL_BITS_UUID, BLEWrite);

// Control bit definitions [which bit in the control_bits byte controls what :) ]
#define CTRL_CAL_START 0
#define CTRL_CAL_DONE 1
#define CTRL_EXER_DONE 2
byte control_bits = 0b00000000;
double pitch_diff = 0.0;
double key_frame_data = 0.0;
byte buf[8] = {0};

void setup() {
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  Serial.println("Successfully initialized Bluetooth® Low Energy module.");

  // Construct the service to be advertised
  central_service.addCharacteristic(key_frame_data_characteristic);
  central_service.addCharacteristic(key_frame_hit_characteristic);
  central_service.addCharacteristic(control_bits_characteristic);
  BLE.addService(central_service);

  // Setup central advertising
  BLE.setLocalName("MoCopy (test)");
  BLE.setAdvertisedService(central_service);
  BLE.advertise();

  Serial.print("Advertising with address: ");
  Serial.println(BLE.address().c_str());
}

void loop() {
  app = BLE.central();
  
  if (app) {
    Serial.print("Connected to app MAC: ");
    Serial.println(app.address());

    while (app.connected()) {
      // Update key_frame_hit with a random bit.
      key_frame_hit_characteristic.writeValue((bool)random(0,2));
      // Read values from the app.
      key_frame_data_characteristic.readValue(&key_frame_data, 8);
      control_bits_characteristic.readValue(control_bits);
      Serial.print("KF.written(): ");
      Serial.print(key_frame_data_characteristic.written());
      Serial.print(", CB.written(): ");
      Serial.print(control_bits_characteristic.written());
      Serial.print(", key_frame_data: ");
      Serial.print(key_frame_data);
      Serial.print(", control_bits: ");
      Serial.print(control_bits);
      Serial.println();
    }

    if (!app.connected()) {
      Serial.print("Disconnected from app MAC: ");
      Serial.println(app.address());
      // Retry connection on next loop.
    }
  }
}
