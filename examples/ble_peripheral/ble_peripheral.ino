// A BLE peripheral (server) device that advertises its services for central (client)
// devices to connect to.
// Install the ArduinoBLE library from the library manager in the IDE.

#include <ArduinoBLE.h>

// REMEMBER! Local to peripheral is peripheral to central! ;)
#define LOCAL_SERVICE_UUID "a3be2240-409d-4d82-ac18-1146c7d30b44"
#define LOCAL_CHARACTERISTIC_UUID "3cefd1a0-69e7-4bc2-a89d-092dbb32d339"

BLEService localService(LOCAL_SERVICE_UUID);
BLEFloatCharacteristic localCharacteristic(LOCAL_CHARACTERISTIC_UUID, BLERead | BLENotify);

BLEDevice central;

char printString [64];
byte buf[4] = {0};

float val = 0.0;

void setup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");

  // Construct the service to be advertised.
  localService.addCharacteristic(localCharacteristic);
  BLE.addService(localService);

  // Setup peripheral advertising.
  BLE.setLocalName("MoCopy Peripheral");
  BLE.setAdvertisedService(localService);
  BLE.advertise();
  Serial.print("Advertising with address: ");
  Serial.println(BLE.address().c_str());
}

void updateBLE() {
  central = BLE.central();
  // Note: The peripheral does not attempt a connection to the central and thus
  // does not call the connect() method.

  if (central) {
    Serial.print("Connected to central MAC: ");
    Serial.println(central.address());

    while (central.connected()) {
      // Call some function here.
      val += 1.0;
      localCharacteristic.setValue(val);
    }

    Serial.println("Disconnected from central MAC: ");
    Serial.println(central.address());
  }
}

void loop() {
  updateBLE();
}
