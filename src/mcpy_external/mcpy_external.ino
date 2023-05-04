// An external device for the MoCopy® system, i.e. the outer most band
// on a particular limb such as the wrist on the arm.
// Install the ArduinoBLE library from the library manager in the IDE.

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <ArduinoBLE.h>

#define DEBUG_PRINT_BLE 1
#define DEBUG_PRINT_STATUS 0

#define EXTERNAL_SERVICE_UUID "56176a63-d563-43f4-b239-636f41b63c6d"
#define JOINT_ORITENTATION_CHARACTERISTIC_UUID "b99cc0f3-8cdc-4bb1-a51d-3927431f0985"
#define EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID "04308b2c-90dc-4984-8c45-81650dff60b8"

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

// ------ Math defines --------

#define SAMPLE_PERIOD_MS 100
#define YAW_GRACE_ANGLE_DEGREES 15
#define PITCH_GRACE_ANGLE_DEGREES 15
#define MAX_VIBRATION 255
#define BASE_VIBRATION 120 // determined experimentally
#define SAMPLE_PERIOD_MS 10

// ------ Math defines --------

BLEService externalService(EXTERNAL_SERVICE_UUID);
BLEFloatCharacteristic jointOrientation(JOINT_ORITENTATION_CHARACTERISTIC_UUID, BLEWrite);
BLEFloatCharacteristic externalOrientation(EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEDevice central;

Adafruit_BNO055 bno;
imu::Vector<3> joint_euler_vector, external_euler_vector, correct_vector, error_vector;
float joint_pitch, external_pitch;

char printString [64];
byte buf[4] = {0};

uint8_t vibes[3] = {0, 0, 0};

void setup() {
  Serial.begin(9600);
  while (!Serial);

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

void updateHardware() {
  external_euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  external_pitch = external_euler_vector[2];

  if (DEBUG_PRINT_BLE) {
    Serial.print("Joint pitch: ");
    Serial.print(joint_pitch);
    Serial.print(" External pitch: ");
    Serial.println(external_pitch);
  }

  error_vector = external_euler_vector - correct_vector;

  // Calculate the vibration strength
  vibes[0] = max(0.0, min((uint8_t)(fabs(error_vector[0])) - YAW_GRACE_ANGLE_DEGREES + BASE_VIBRATION, MAX_VIBRATION));
  vibes[2] = max(0.0, min((uint8_t)(2 * fabs(error_vector[2])) - PITCH_GRACE_ANGLE_DEGREES + BASE_VIBRATION, MAX_VIBRATION));

  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);

  if (error_vector[0] >= YAW_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Right, ");
    analogWrite(RIGHT_MOTOR, vibes[0]);
    analogWrite(LEFT_MOTOR, 0);
    digitalWrite(RED, HIGH);
  } else if (error_vector[0] <= -YAW_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Left, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, vibes[0]);
    digitalWrite(GREEN, HIGH);
  } else {
    if (DEBUG_PRINT_STATUS) Serial.print("Grace, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, 0);
    digitalWrite(BLUE, HIGH);
  }

  if (error_vector[2] >= PITCH_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Down, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, vibes[2]);
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, HIGH);
  } else if (error_vector[2] <= -PITCH_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Up, ");
    analogWrite(UP_MOTOR, vibes[2]);
    analogWrite(DOWN_MOTOR, 0);
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, HIGH);
  } else {
    if (DEBUG_PRINT_STATUS) Serial.print("Grace, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, 0);
    digitalWrite(GREEN, HIGH);
    digitalWrite(BLUE, HIGH);
  } 

  delay(SAMPLE_PERIOD_MS);
}

void updateBLE() {
  central = BLE.central();
  // Note: The peripheral does not attempt a connection to the central and thus
  // does not call the connect() method.

  if (central) {
    Serial.print("Connected to central MAC: ");
    Serial.println(central.address());

    while (central.connected()) {
      updateHardware();
      jointOrientation.readValue(buf, 4);
      memcpy(&joint_pitch, buf, 4);
      externalOrientation.setValue(external_pitch);
    }

    Serial.println("Disconnected from central MAC: ");
    Serial.println(central.address());
  }
}

void loop() {
  updateBLE();
}
