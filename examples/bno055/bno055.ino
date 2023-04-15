#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <string>

// Global Variables
Adafruit_BNO055 bno;
imu::Vector<3> euler_vector, gravity_vector;
imu::Vector<3> desired_vector, error_vector;
sensors_event_t bno_event;
char print_str[100];

void setup() {
  Serial.begin(115200);
  // Try to initialize the IMU
  if (!bno.begin(OPERATION_MODE_NDOF)) {
		Serial.println("Failed to find BNO055 chip");
		while (1) {
		  delay(100);
		}
	}
	Serial.println("BNO055 Found!");
  bno.enterNormalMode();

  // Temporary setup
  desired_vector = {0.0, 0.0, 0.0};
}

void loop() {
  // imu.getEvent(&imu_event, VECTOR_EULER);
  euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  error_vector = euler_vector - desired_vector;
  sprintf(
    print_str,
    "Euler Vector <%lf, %lf, %lf>",
    euler_vector[0],
    euler_vector[1],
    euler_vector[2]);
  Serial.println(print_str);

/*
  sprintf(
    print_str,
    "Error Vector <%lf, %lf, %lf>",
    error_vector[0],
    error_vector[1],
    error_vector[2]);
  Serial.println(print_str);
*/

  delay(250);
}
