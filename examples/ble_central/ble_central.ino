// A BLE central (client) device that scans for peripheral (server) devices to connect to.
// Install the ArduinoBLE library from the library manager in the IDE.

#include <ArduinoBLE.h>

#define ADVERT_UUID_BYTES 16 // 128 bits
#define STD_SERVICE_UUID_BYTES 2 // 16 bits
#define CUSTOM_SERVICE_UUID_BYTES 16 // 128 bits
#define CHARACTERISTIC_VALUE_BYTES 512

#define YAW_CHARACTERISTIC_UUID "41e0f662-7303-4ae6-94be-3b5d3391caad"

BLEService imu_service(SERVICE_UUID);
BLEFloatCharacteristic yaw_characteristic(CHARACTERISTIC_VALUE_BYTES, BLE_READ | BLE_NOTIFY);
BLEDevice device;

void setup() {
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");
  
  BLE.setLocalName("MoCopy Central");

  do {
    BLE.scanForUuid(ACCEL_UUID);
    peripheral = BLE.available();
    Serial.print(".");
    delay(100);
  } while (!peripheral);
}

void loop() {

}
