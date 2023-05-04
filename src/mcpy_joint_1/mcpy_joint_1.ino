// MoCopy joint device (between external and central).
// Install the ArduinoBLE library from the library manager in the IDE.

#include <string>
#include <math.h>
#include <ArduinoBLE.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// #define LOCAL_SERVICE_UUID "c2c727d2-0f5c-4c4b-b2de-27b8dbb64e13"
// #define LOCAL_CHARACTERISTIC_UUID "41e0f662-7303-4ae6-94be-3b5d3391caad"

#define EXTERNAL_SERVICE_UUID "56176a63-d563-43f4-b239-636f41b63c6d"
#define JOINT_ORITENTATION_CHARACTERISTIC_UUID "b99cc0f3-8cdc-4bb1-a51d-3927431f0985"
#define EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID "04308b2c-90dc-4984-8c45-81650dff60b8"

BLEService externalService(EXTERNAL_SERVICE_UUID);
// BLEFloatCharacteristic localCharacteristic(LOCAL_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEDevice central, peripheral;

Adafruit_BNO055 bno;
imu::Vector<3> joint_vector, external_vector;

float joint_pitch, external_pitch;
char printString [64];
byte buf[4] = {0};

void setup() {
  Serial.begin(9600);

  while (!Serial);

  if (!bno.begin(OPERATION_MODE_NDOF)) {
		Serial.println("\nFailed to find BNO055 chip");
		while (1) {
		  delay(10);
		}
	}
	Serial.println("\nBNO055 Found!");
  bno.enterNormalMode();

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");

  // // Construct the service to be advertised.
  // localService.addCharacteristic(localCharacteristic);
  // BLE.addService(localService);

  // // Setup peripheral advertising.
  // BLE.setLocalName("MoCopy Central-Peripheral");
  // BLE.setAdvertisedService(localService);
  // BLE.advertise();

  // Serial.print("Advertising with address: ");
  // Serial.println(BLE.address().c_str());

  // Scan for peripheral services.
  Serial.println("Scanning for external service");
  do {
    BLE.scanForUuid(EXTERNAL_SERVICE_UUID);
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

void updateBLE() {
  // central = BLE.central();

  // if (central) {
  //   Serial.print("Connected to central MAC: ");
  //   Serial.println(central.address());

    // Maybe change this to a for loop to make X attempts before quitting.
    if (!peripheral.connected()) {
      if (peripheral.connect()) {
        Serial.println("Successfully connected to peripheral.");
      } else {
        Serial.println("Failed to connect to peripheral.");
        Serial.print("Device MAC address: ");
        Serial.println(peripheral.address());
        Serial.print("Device Name: ");
        Serial.println(peripheral.localName());
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

    BLECharacteristic jointOrientation = peripheral.characteristic(JOINT_ORITENTATION_CHARACTERISTIC_UUID);
    BLECharacteristic externalOrientation = peripheral.characteristic(EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID);

    if (!jointOrientation || !externalOrientation) {
      Serial.println("Peripheral device does not have the expected characteristic(s).");
      peripheral.disconnect();
      return;
    } else if (!externalOrientation.canSubscribe()) { // jointOrientation is BLEWrite threfore can't subscribe.
      Serial.println("Cannot subscribe to the peripheral device's characteristic(s).");
      peripheral.disconnect();
      return;
    }

    // while (central.connected() && peripheral.connected()) {
    while (peripheral.connected()) {
      joint_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
      joint_pitch = joint_vector[2];
      memcpy(buf, &joint_pitch, 4);
      jointOrientation.setValue(buf, 4);

      externalOrientation.readValue(buf, 4);
      memcpy(&external_pitch, buf, 4);
    }

    // if (!central.connected()) {
    //   Serial.print("Disconnected from central MAC: ");
    //   Serial.println(central.address());
    // }

    if (!peripheral.connected()) {
      Serial.print("Disconnected from peripheral MAC: ");
      Serial.println(peripheral.address());
    }
  // }
}

void loop() {
  Serial.println("in loop");
  updateBLE();
}
