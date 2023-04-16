#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <string>
// #include <stdlib.h>
#include <math.h>
// #include <ESP8266WiFi.h>

#define SAMPLE_PERIOD_MS 100

// RGB LED
#define RED D0
#define GREEN D3
#define BLUE TX

// Motors
#define M1 D5
#define M2 D6
#define M3 D7
#define M4 D8
#define UP_MOTOR D5
#define DOWN_MOTOR D8
#define LEFT_MOTOR D6
#define RIGHT_MOTOR D7

int count = 0;

// Global Variables
Adafruit_BNO055 bno;
imu::Vector<3> euler_vector, correct_vector, error_vector;
sensors_event_t imu_event;
char print_str[512];

void setup() {
  Serial.begin(115200);
  // Try to initialize the IMU
  if (!bno.begin()) {
		Serial.println("\nFailed to find BNO055 chip");
		while (1) {
		  delay(10);
		}
	}
	Serial.println("\nBNO055 Found!");
  Serial.println("checkpoint 0");
  // bno.enterNormalMode();
  Serial.println("checkpoint 1");

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  Serial.println("checkpoint 2");
  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
  Serial.println("checkpoint 3");
  
  analogWriteResolution(8);
  Serial.println("checkpoint 4");
  pinMode(UP_MOTOR, OUTPUT);
  pinMode(DOWN_MOTOR, OUTPUT);
  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);
  Serial.println("checkpoint 5");

  correct_vector = {0, 0, 0};
  Serial.println("checkpoint 6");
}

void loop() {
  Serial.println("In loop!");

  euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  // sprintf(
  //   print_str,
  //   "Euler Vector <%f, %f, %f>",
  //   euler_vector[0],
  //   euler_vector[1],
  //   euler_vector[2]
  // );
  // Serial.println(print_str);

  error_vector = euler_vector - correct_vector;
  sprintf(
    print_str,
    "Euler Vector <%f, %f, %f>",
    error_vector[0],
    error_vector[1],
    error_vector[2]
  );
  Serial.println(print_str);

  if (error_vector[0] >= 0) {
    analogWrite(RIGHT_MOTOR, error_vector[0]);
    analogWrite(LEFT_MOTOR, 0);
  } else {
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, fabs(error_vector[0]));
  }

  if (error_vector[1] >= 0) {
    analogWrite(UP_MOTOR, error_vector[1]);
    analogWrite(DOWN_MOTOR, 0);
  } else {
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, fabs(error_vector[1]));
  }
}