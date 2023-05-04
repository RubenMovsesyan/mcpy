#include <ArduinoBLE.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <string>
#include <math.h>

// --------------------------- BLE defines -------------------------

#define JOINT_SERVICE_UUID "a9a95e92-26ea-4282-bd0c-7c8bd6c65a2b"
#define EXTERNAL_SERVICE_UUID "56176a63-d563-43f4-b239-636f41b63c6d"

// Characteristic UUIDs
// local uuids
#define REP_COMPLETION_CHARACTERISTIC_UUID "08d54caf-75bc-4aa6-876b-8eea5427605a"

// external uuids
#define JOINT_ORITENTATION_CHARACTERISTIC_UUID "b99cc0f3-8cdc-4bb1-a51d-3927431f0985"
#define EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID "04308b2c-90dc-4984-8c45-81650dff60b8"

// --------------------------- BLE defines -------------------------

// --------------------------- Hardware defines --------------------

// RGB LED
#define RED             D2
#define GREEN           D3
#define BLUE            D4

// Motors
#define UP_MOTOR        D9
#define DOWN_MOTOR      D8
#define LEFT_MOTOR      D7
#define RIGHT_MOTOR     D6

// --------------------------- Hardware defines --------------------

// ------ Other defines -------

// #define

// ------ Other defines -------

// ---- Hardware functions ----

uint8_t min(uint8_t a, uint8_t b) {if (a <= b) return a; else return b; }
uint8_t max(uint8_t a, uint8_t b) {if (a >= b) return a; else return b; }

// ---- Hardware functions ----

// ----- Global Variables -----

// IMU variables
Adafruit_BNO055 bno;
imu::Vector<3> euler_vector, correct_vector, error_vector, gravity_vector;
// Temp by Anthony
imu::Vector<3> joint_euler_vector, external_euler_vector;
float joint_pitch = 0.0;
sensors_event_t imu_event;
// char print_str[CHAR_SIZE];

// BLE variables
BLEDevice peripheral;
BLEService joint_service(JOINT_SERVICE_UUID);
BLEService externalService(EXTERNAL_SERVICE_UUID);
BLEFloatCharacteristic rep_completion_characteristic(REP_COMPLETION_CHARACTERISTIC_UUID, BLERead);

byte buf[4] = {0};

// Hardware variables
/*
                      (
                     (  ) (
                      )    )
         |||||||     (  ( (
        ( O   O )        )
 ____oOO___(_)___OOo____(
(_______________________)
*/
uint8_t vibes[3] = {0, 0, 0};

// ----- Global Variables -----


// initialize the BNO055 and all the pins
void initHardware() {
  // start the bno
  if (!bno.begin()) {
    Serial.println("\nFailed to find BNO055 chip");
    while (1);
  }

  Serial.println("\nBNO055 Found!");
  bno.enterNormalMode();

  // init rgb pins
  pinMode(RED,          OUTPUT);
  pinMode(GREEN,        OUTPUT);
  pinMode(BLUE,         OUTPUT);
  digitalWrite(RED,     LOW);
  digitalWrite(GREEN,   LOW);
  digitalWrite(BLUE,    LOW);

  // init motor pins
  analogWriteResolution(8);
  pinMode(UP_MOTOR,     OUTPUT);
  pinMode(DOWN_MOTOR,   OUTPUT);
  pinMode(LEFT_MOTOR,   OUTPUT);
  pinMode(RIGHT_MOTOR,  OUTPUT);

  Serial.println("Mcpy hardware initialized");

  correct_vector = {180, 0, 0};
}

// initialize the BLE
void initBLE() {
  if (!BLE.begin()) {
    Serial.println("* Starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  Serial.println("Successfully initialized Bluetooth® Low Energy module.");

  // set up hosting service
  BLE.setLocalName("Mcpy (joint)");
  // BLE.setAdvertisedService(joint_service);
  // joint_service.addCharacteristic(rep_completion_characteristic);
  // BLE.addService(joint_service);
  // rep_completion_characteristic.setValue(0.0f);

  // Serial.println("* Mcpy BLE setup complete (joint device)");

  // BLE.advertise();

  // Serial.print("* Server address: ");
  // Serial.println(BLE.address().c_str());

  // Initialize connection to peripheral (external band).
  do {
    BLE.scanForUuid(EXTERNAL_SERVICE_UUID);
    peripheral = BLE.available();
  } while (!peripheral);

  Serial.println("Peripheral device found.");
  Serial.print("Device MAC address: ");
  Serial.println(peripheral.address());
  Serial.print("Device Name: ");
  Serial.println(peripheral.localName());

  if (peripheral.discoverAttributes()) {
    Serial.println("Successfully discovered peripheral attributes.");
  } else {
    Serial.println("Failed to discover peripheral attributes.");
    peripheral.disconnect();
    return;
  }

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

// updates the vectors and computes the error and vibrations
// based on the correct vector
void updateHardware() {
  
}

void setup() {
  Serial.begin(9600);

}

void loop() {
  if (!peripheral.connected()) {
    if (peripheral.connect()) {
      Serial.println("Successfully connected to peripheral.");
    } else {
      Serial.println("Failed to connect to peripheral.");
      Serial.print("Peripheral MAC address: ");
      Serial.println(peripheral.address());
      Serial.print("Peripheral Name: ");
      Serial.println(peripheral.localName());
      return;
    }
  }

  BLECharacteristic jointOrientation = peripheral.characteristic(JOINT_ORITENTATION_CHARACTERISTIC_UUID);
  BLECharacteristic externalOrientation = peripheral.characteristic(EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID);

  if (!jointOrientation || !externalOrientation) {
    Serial.println("Peripheral device does not have the expected characteristic(s).");
    peripheral.disconnect();
    return;
  } else if (!jointOrientation.canSubscribe() || !externalOrientation.canSubscribe()) {
    Serial.println("Cannot subscribe to the peripheral device's characteristic(s).");
    peripheral.disconnect();
    return;
  }

  // while (central.connected() && peripheral.connected()) {
    while (peripheral.connected()) {
      // Call some function here.
      joint_euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
      joint_pitch = joint_euler_vector[2];
      memcpy(&joint_pitch, buf, 4);
      jointOrientation.setValue(buf, 4);

      externalOrientation.readValue(buf, 4);
      memcpy(&(external_euler_vector[2]), buf, 4);
    }
}
