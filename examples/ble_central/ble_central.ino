// A BLE central (client) device that scans for peripheral (server) devices to connect to.
// Install the ArduinoBLE library from the library manager in the IDE.

#include <ArduinoBLE.h>

#define EXERCISE_SERVICE_UUID "a3be2240-409d-4d82-ac18-1146c7d30b44"
#define CORRECT_YAW_CHARACTERISTIC_UUID "41e0f662-7303-4ae6-94be-3b5d3391caad"

BLEService exerciseService(EXERCISE_SERVICE_UUID);

// convention: name this device "device", this device's central "central" and this device's
// peripherals "peripheral_{i}"
BLEDevice peripheral;

char printString [64];
byte buf[4] = {0};

void setup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");
  BLE.setLocalName("MoCopy Central");

  do {
    BLE.scanForUuid(EXERCISE_SERVICE_UUID);
    peripheral = BLE.available(); // .peripheral(); instead?
    Serial.print(".");
    delay(100);
  } while (!peripheral);

  Serial.println("Peripheral device found.");
  Serial.print("Device MAC address: ");
  Serial.println(peripheral.address());
  Serial.print("Device Name: ");
  Serial.println(peripheral.localName());

  // Print all services advertised by the peripheral.
  if (peripheral.hasAdvertisedServiceUuid()) {
    Serial.println("Peripheral's advertised services UUIDs:");
    for (int i = 0; i < peripheral.advertisedServiceUuidCount(); i++) {
      Serial.print(peripheral.advertisedServiceUuid(i));
      Serial.print(", ");
    }
    Serial.println();
  } else {
    Serial.println("Peripheral has no advertised services!");
  }

  BLE.stopScan();
}

void loop() {
  // Maybe change this to a for loop to make X attempts before quitting.
  if (peripheral.connect()) {
    Serial.println("Successfully connected to peripheral.");
  } else {
    Serial.println("Failed to connect to periphal.");
    return;
  }

  if (peripheral.discoverAttributes()) {
    Serial.println("Successfully discovered peripheral attributes.");
  } else {
    Serial.println("Failed to discover peripheral attributes.");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic correctYawCharacteristic = peripheral.characteristic(CORRECT_YAW_CHARACTERISTIC_UUID);

  if (!correctYawCharacteristic) {
    Serial.println("Peripheral device does not have the expected characteristic.");
    peripheral.disconnect();
    return;
  } else if (!correctYawCharacteristic.canSubscribe()) {
    Serial.println("Cannot subscribe to the peripheral device's characteristic.");
    peripheral.disconnect();
    return;
  }

  float val;

  while (peripheral.connected()) {
    correctYawCharacteristic.readValue(buf, 4);
    memcpy(&val, buf, 4);
    sprintf(printString, "%f", val);
    Serial.println(printString);
  }
  // If the device disconnects, it will reattempt this entire connection process.
}
