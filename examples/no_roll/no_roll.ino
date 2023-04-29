#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <string>
// #include <stdlib.h>
#include <math.h>
// #include <ESP8266WiFi.h>

// <0 to 360, ? to ?, -90 to 90>

// Debug print defines. Only enable one for best results.
#define DEBUG_PRINT_STATUS 0
#define DEBUG_PRINT_EULER 1

#define SAMPLE_PERIOD_MS 100
#define YAW_GRACE_ANGLE_DEGREES 15
#define PITCH_GRACE_ANGLE_DEGREES 15
#define MAX_VIBRATION 255
// #define BASE_VIBRATION 150 // determined experimentally
#define BASE_VIBRATION 120 // determined experimentally

// RGB LED
#define RED D0
#define GREEN D3
#define BLUE TX

// Motors
// (upside down)
// #define UP_MOTOR D6
// #define DOWN_MOTOR D8
// #define LEFT_MOTOR D7
// #define RIGHT_MOTOR D5

#define UP_MOTOR D7
#define DOWN_MOTOR D5
#define LEFT_MOTOR D8
#define RIGHT_MOTOR D6

// Global Variables
Adafruit_BNO055 bno;
imu::Vector<3> euler_vector, correct_vector, error_vector;
sensors_event_t imu_event;
char print_str[512];

int count = 0;
uint8_t vibes[3] = {0, 0 ,0};

uint8_t min(uint8_t a, uint8_t b) { if (a <= b) return a; else return b; };
uint8_t max(uint8_t a, uint8_t b) { if (a >= b) return a; else return b; };

void setup() {
  Serial.begin(9600);
  // Try to initialize the IMU
  if (!bno.begin()) {
		Serial.println("\nFailed to find BNO055 chip");
		while (1) {
		  delay(10);
		}
	}
	Serial.println("\nBNO055 Found!");
  bno.enterNormalMode();

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  // pinMode(BLUE, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  // digitalWrite(BLUE, LOW);
  
  analogWriteResolution(8);
  pinMode(UP_MOTOR, OUTPUT);
  pinMode(DOWN_MOTOR, OUTPUT);
  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);

  correct_vector = {180, 0, 0};
}

void loop() {
  euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  if (DEBUG_PRINT_EULER) {
    sprintf(
      print_str,
      "Euler Vector <%f, %f, %f>",
      euler_vector[0],
      euler_vector[1],
      euler_vector[2]
    );
    Serial.println(print_str);
  }

  error_vector = euler_vector - correct_vector;
  // error_vector[0] = error_vector[0] - 180.0; // get within a range of -180 to +180
  // sprintf(
  //   print_str,
  //   "Error Vector <%f, %f, %f>",
  //   error_vector[0],
  //   error_vector[1],
  //   error_vector[2]
  // );
  // Serial.println(print_str);

  // Calculate vibration strength.
  vibes[0] = max(0.0, min((uint8_t)(fabs(error_vector[0])) - YAW_GRACE_ANGLE_DEGREES + BASE_VIBRATION, MAX_VIBRATION));
  vibes[2] = max(0.0, min((uint8_t)(2 * fabs(error_vector[2])) - PITCH_GRACE_ANGLE_DEGREES + BASE_VIBRATION, MAX_VIBRATION));

  if (error_vector[0] >= YAW_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Right, ");
    analogWrite(RIGHT_MOTOR, vibes[0]);
    analogWrite(LEFT_MOTOR, 0);
  } else if (error_vector[0] <= -YAW_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Left, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, vibes[0]);
  } else {
    if (DEBUG_PRINT_STATUS) Serial.print("Grace, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, 0);
  }

  if (error_vector[2] >= PITCH_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Down, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, vibes[2]);
  } else if (error_vector[2] <= -PITCH_GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_STATUS) Serial.print("Up, ");
    analogWrite(UP_MOTOR, vibes[2]);
    analogWrite(DOWN_MOTOR, 0);
  } else {
    if (DEBUG_PRINT_STATUS) Serial.print("Grace, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, 0);
  }

  // sprintf(print_str, "vibes[2]: %d", vibes[2]);
  // Serial.print(print_str) ; 

  if (DEBUG_PRINT_STATUS) Serial.print("\n");

  delay(SAMPLE_PERIOD_MS);
}