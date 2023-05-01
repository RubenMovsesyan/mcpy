// A BLE peripheral (server) device that advertises its services for central (client)
// devices to connect to.
// Install the ArduinoBLE library from the library manager in the IDE.

#include <ArduinoBLE.h>

#define EXERCISE_SERVICE_UUID "a3be2240-409d-4d82-ac18-1146c7d30b44"
#define CORRECT_YAW_CHARACTERISTIC_UUID "41e0f662-7303-4ae6-94be-3b5d3391caad"

BLEService exerciseService(EXERCISE_SERVICE_UUID);
BLEFloatCharacteristic correctYawCharacteristic(CORRECT_YAW_CHARACTERISTIC_UUID, BLERead | BLENotify);

char printString [64];
byte buf[4] = {0};

void setup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");

  // Construct the service to be advertised.
  exerciseService.addCharacteristic(correctYawCharacteristic);
  BLE.addService(exerciseService);

  // Setup peripheral advertising.
  BLE.setLocalName("MoCopy Peripheral");
  BLE.setAdvertisedService(exerciseService);
  BLE.advertise();
  Serial.print("Advertising with address: ");
  Serial.println(BLE.address().c_str());
}

void updateBLE() {
  BLEDevice central = BLE.central();
  // Note: The peripheral does not attempt a connection to the central and thus
  // does not call the connect() method.

  if (central) {
    Serial.print("Connected to central MAC: ");
    Serial.println(central.address());

    while (central.connected()) {
      // Call some function here.
      correctYawCharacteristic.setValue((float)random());
    }

    Serial.println("Disconnected from central MAC: ");
    Serial.println(central.address());
  }
}

void loop() {
  updateBLE();
}
