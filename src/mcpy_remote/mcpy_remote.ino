#include <ArduinoBLE.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <string>
#include <math.h>

// Debug print defines. Only enable one for best results.
#define DEBUG_PRINT_STATUS 0
#define DEBUG_PRINT_EULER 1

#define SAMPLE_PERIOD_MS 10
#define YAW_GRACE_ANGLE_DEGREES 15
#define PITCH_GRACE_ANGLE_DEGREES 15
#define MAX_VIBRATION 255
#define BASE_VIBRATION 120 // determined experimentally

// RGB LED
#define RED D2
#define GREEN D3
#define BLUE D4

// Motors
#define UP_MOTOR D9
#define DOWN_MOTOR D8
#define LEFT_MOTOR D7
#define RIGHT_MOTOR D6

// Other Global Defines
#define CHAR_SIZE 128

// TODO: get this to work properly
// macros
// #define min(a, b) (a <= b) ? a : b;
// #define max(a, b) (a >= b) ? a : b;

// BLE defines
#define SERVICE_UUID "0b03321e-e6ad-11ed-a05b-0242ac120003"
#define CHARACTERISTIC_UUID "1a01ecfc-e6b1-11ed-a05b-0242ac120003"

// Hardware functions
uint8_t min(uint8_t a, uint8_t b) { if (a <= b) return a; else return b; };
uint8_t max(uint8_t a, uint8_t b) { if (a >= b) return a; else return b; };

// Global Variables >=(
Adafruit_BNO055 bno;
imu::Vector<3> euler_vector, correct_vector, error_vector;
sensors_event_t imu_event;
char print_str[CHAR_SIZE];

// BLE variables
static const char* greeting = "Hola, Mundo!";
BLEService remoteService(SERVICE_UUID);
BLEStringCharacteristic accelCharacteristic(CHARACTERISTIC_UUID, BLERead | BLENotify, CHAR_SIZE);

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

// initialize the BNO055 and all the pins
void initHardware() {
  if (!bno.begin()) {
    Serial.println("\nFailed to find BNO055 chip");
    while(1);
  }

  Serial.println("\nBNO055 Found!");
  bno.enterNormalMode();

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);

  analogWriteResolution(8);
  pinMode(UP_MOTOR, OUTPUT);
  pinMode(DOWN_MOTOR, OUTPUT);
  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);

  Serial.println("Mcpy hardware initialized");

  correct_vector = {180, 0, 0};
}

void initBLE() {
  if (!BLE.begin()) {
    Serial.println("* Starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  BLE.setLocalName("Mcpy (remote)");
  BLE.setAdvertisedService(remoteService);
  remoteService.addCharacteristic(accelCharacteristic);
  BLE.addService(remoteService);
  accelCharacteristic.setValue(greeting);

  Serial.println("Mcpy BLE setup complete (remote device)\n");

  BLE.advertise();
}

// updates the vectors and computes the error and vibrations
// based on the correct vector
void updateHardware() {
  euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  if (DEBUG_PRINT_EULER) {
    sprintf(print_str,
      "EuV <%f, %f, %f>",
      euler_vector[0],
      euler_vector[1],
      euler_vector[2]
    );
    Serial.println(print_str);
  }

  error_vector = euler_vector - correct_vector;

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

  if (DEBUG_PRINT_STATUS) Serial.print("\n");

  delay(SAMPLE_PERIOD_MS);
}

// There is probably a better way to do this than calling
// updateHardware from inside updateBLE
// oh well ¯\_(ツ)_/¯
void updateBLE() {
  BLEDevice peripheral = BLE.central();

  if (peripheral) {
    Serial.print("Connected to peripheral MAC: ");
    Serial.println(peripheral.address());

    while (peripheral.connected()) {
      updateHardware();
      accelCharacteristic.setValue(print_str);
    }

    Serial.println("Disconnected from central MAC: ");
    Serial.println(peripheral.address());
  }
}

// Arduino Functions \/ ----------------------------------------- \/
void setup() {
  Serial.begin(9600);
  while (!Serial);

  initHardware();
  initBLE();
}

void loop() {
  updateBLE();
}
