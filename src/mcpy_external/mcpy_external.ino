// An external device for the MoCopy® system, i.e. the outer most band
// on a particular limb such as the wrist on the arm.
// Install the ArduinoBLE library from the library manager in the IDE.

#include <ArduinoBLE.h>

#define EXTERNAL_SERVICE_UUID "56176a63-d563-43f4-b239-636f41b63c6d"
#define JOINT_ORITENTATION_CHARACTERISTIC_UUID "b99cc0f3-8cdc-4bb1-a51d-3927431f0985"
#define EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID "04308b2c-90dc-4984-8c45-81650dff60b8"

BLEService externalService(EXTERNAL_SERVICE_UUID);
BLEFloatCharacteristic jointCharacteristic(JOINT_ORITENTATION_CHARACTERISTIC_UUID, BLEWrite);
BLEFloatCharacteristic externalCharacteristic(EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEDevice central;

Adafruit_BNO055 bno;
imu::Vector<3> euler_vector, correct_vector, error_vector;

char printString [64];
byte buf[4] = {0};

// float yaw = 0.0;
float external_pitch = 0.0;
// float roll = 0.0;

float joint_pitch = 0.0;

void setup() {
  Serial.begin(9600);

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");

  // Construct the service to be advertised.
  externalService.addCharacteristic(jointCharacteristic);
  externalService.addCharacteristic(externalCharacteristic);
  BLE.addService(externalService);

  // Setup peripheral advertising.
  BLE.setLocalName("MoCopy External");
  BLE.setAdvertisedService(externalService);
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
      jointCharacteristic.readValue(buf, 4);
      memcpy(&joint_pitch, buf, 4);
      externalCharacteristic.setValue(external_pitch);
    }

    Serial.println("Disconnected from central MAC: ");
    Serial.println(central.address());
  }
}

void loop() {
  updateBLE();
}
