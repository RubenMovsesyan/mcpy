// This BLE device operates as both a central (client) and peripheral (server) to
// two or more other devices in a mesh network topology. In essence, this device is
// a single "link" in a daisy-chain of BLE devices.
// Install the ArduinoBLE library from the library manager in the IDE.

// There is an ordering to how this device connects to others.
// First, it advertises its own services as a peripheral,
// Second, it scans for services provided by a different (peripheral) device.
// Third, if both of the previous two steps succeed then the device should
// first confirm that the central connected to it and then explicitly connect
// to the its peripheral.

#include <ArduinoBLE.h>

// Local UUIDs
#define LOCAL_SERVICE_UUID "c2c727d2-0f5c-4c4b-b2de-27b8dbb64e13"
#define LOCAL_CHARACTERISTIC_UUID "41e0f662-7303-4ae6-94be-3b5d3391caad"

// Peripheral UUIDs
#define PERIPHERAL_SERVICE_UUID "a3be2240-409d-4d82-ac18-1146c7d30b44"
#define PERIPHERAL_CHARACTERISTIC_UUID "3cefd1a0-69e7-4bc2-a89d-092dbb32d339"

BLEService localService(LOCAL_SERVICE_UUID);
BLEFloatCharacteristic localCharacteristic(LOCAL_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEDevice central, peripheral;

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
  localService.addCharacteristic(localCharacteristic);
  BLE.addService(localService);

  // Setup peripheral advertising.
  BLE.setLocalName("MoCopy Central-Peripheral");
  BLE.setAdvertisedService(localService);
  BLE.advertise();

  Serial.print("Advertising with address: ");
  Serial.println(BLE.address().c_str());

  // Scan for peripheral services.
  do {
    BLE.scanForUuid(PERIPHERAL_SERVICE_UUID);
    peripheral = BLE.available();
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

void chainPeripheral(BLECharacteristic characteristic) {
  float info;

  if (peripheral.connected()) {
    characteristic.readValue(buf, 4);
    memcpy(&info, buf, 4);
    localCharacteristic.setValue(info);
  } 
  // else {
  //   Serial.println("Peripheral Disconnected");
  //   if (peripheral.connect()) {
  //     Serial.println("Successfully connected to peripheral.");
  //   } else {
  //     Serial.println("Failed to connect to peripheral.");
  //     return;
  //   }
  // }
}

void updateBLE() {
  central = BLE.central();

  if (central) {
    Serial.print("Connected to central MAC: ");
    Serial.println(central.address());

    // Maybe change this to a for loop to make X attempts before quitting.
    if (!peripheral.connected()) {
      if (peripheral.connect()) {
        Serial.println("Successfully connected to peripheral.");
      } else {
        Serial.println("Failed to connect to peripheral.");
        return;
      }
    }

    if (peripheral.discoverAttributes()) {
      Serial.println("Successfully discovered peripheral attributes.");
    } else {
      Serial.println("Failed to discover peripheral attributes.");
      peripheral.disconnect();
      return;
    }

    BLECharacteristic peripheralCharacteristic = peripheral.characteristic(PERIPHERAL_CHARACTERISTIC_UUID);

    if (!peripheralCharacteristic) {
      Serial.println("Peripheral device does not have the expected characteristic.");
      peripheral.disconnect();
      return;
    } else if (!peripheralCharacteristic.canSubscribe()) {
      Serial.println("Cannot subscribe to the peripheral device's characteristic.");
      peripheral.disconnect();
      return;
    }

    while (central.connected() && peripheral.connected()) {
      // Call some function here.
      chainPeripheral(peripheralCharacteristic);
      // localCharacteristic.setValue((float)random());
    }

    if (!central.connected()) {
      Serial.print("Disconnected from central MAC: ");
      Serial.println(central.address());
    }

    if (!peripheral.connected()) {
      Serial.print("Disconnected from peripheral MAC: ");
      Serial.println(peripheral.address());
    }
  }
}

void loop() {
  updateBLE();
}
