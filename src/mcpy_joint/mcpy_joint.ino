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
sensors_event_t imu_event;
// char print_str[CHAR_SIZE];

// BLE variables
BLEService joint_service(JOINT_SERVICE_UUID);
BLEFloatCharacteristic rep_completion_characteristic(REP_COMPLETION_CHARACTERISTIC_UUID);

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

  // set up hosting service
  BLE.setLocalName("Mcpy (joint)");
  BLE.setAdvertisedService(joint_service);
  joint_service.addCharacteristic(rep_completion_characteristic);
  BLE.addService(joint_service);
  rep_completion_characteristic.setValue(0.0f);

  Serial.println("* Mcpy BLE setup complete (joint device)\n");

  BLE.advertise();

  Serial.print("* Server address: ");
  Serial.println(BLE.address().c_str());
}

// updates the vectors and computes the error and vibrations
// based on the correct vector
void updateHardware() {
  
}

void setup() {
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

}
