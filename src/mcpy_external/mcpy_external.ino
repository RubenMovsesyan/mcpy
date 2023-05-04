// An external device for the MoCopy® system, i.e. the outer most band
// on a particular limb such as the wrist on the arm.
// Install the ArduinoBLE library from the library manager in the IDE.

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <ArduinoBLE.h>

#define EXTERNAL_SERVICE_UUID "56176a63-d563-43f4-b239-636f41b63c6d"
#define JOINT_ORITENTATION_CHARACTERISTIC_UUID "b99cc0f3-8cdc-4bb1-a51d-3927431f0985"
#define EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID "04308b2c-90dc-4984-8c45-81650dff60b8"

#define SAMPLE_PERIOD_MS 100
#define YAW_GRACE_ANGLE_DEGREES 15
#define PITCH_GRACE_ANGLE_DEGREES 15

BLEService externalService(EXTERNAL_SERVICE_UUID);
BLEFloatCharacteristic jointOrientation(JOINT_ORITENTATION_CHARACTERISTIC_UUID, BLEWrite);
BLEFloatCharacteristic externalOrientation(EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEDevice central;

Adafruit_BNO055 bno;
imu::Vector<3> joint_euler_vector, external_euler_vector, correct_vector, error_vector;
float joint_pitch, external_pitch;

char printString [64];
byte buf[4] = {0};

void setup() {
  Serial.begin(9600);

  // Try to initialize the IMU
  if (!bno.begin(OPERATION_MODE_NDOF)) {
		Serial.println("\nFailed to find BNO055 chip");
		while (1);
	}
	Serial.println("\nBNO055 Found!");
  bno.enterNormalMode();

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while(1);
  }
  Serial.println("Bluetooth® Low Energy module initialized.");

  // Construct the service to be advertised.
  externalService.addCharacteristic(jointOrientation);
  externalService.addCharacteristic(externalOrientation);
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
      jointOrientation.readValue(buf, 4);
      memcpy(&joint_pitch, buf, 4);

      external_euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
      external_pitch = external_euler_vector[2];
      externalOrientation.setValue(external_pitch);
    }

    Serial.println("Disconnected from central MAC: ");
    Serial.println(central.address());
  }
}

void loop() {
  updateBLE();
}
