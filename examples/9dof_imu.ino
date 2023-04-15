/*
sketch_9dof_imu.ino
-------------------
An example demonstrating data retreival from the ESP8266 board using I2C to
communicate with the BNO055 9-DOF IMU board. From the Arduino IDE you must
select the "lolin(wemos) d1 r2 & mini" board which may be found under the
ESP8266 category.

If your computer is having issues communicating with the device over USB
then check that you have CH340/CH341 USB Serial drivers installed correctly.
Moreover, you may be unable to connect if the board is experiencing
overcurrent and shutting itself down for protection. Most likely, this means
you have connected the BNO055 to the 3.3V pin of the ESP8266 rather than the
5.0V pin.
*/
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <string>

// Global Variables
Adafruit_BNO055 bno;
imu::Vector<3> euler_vector, gravity_vector;
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
}

void loop() {
  // imu.getEvent(&imu_event, VECTOR_EULER);
  euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  sprintf(
    print_str,
    "Euler Vector <%lf, %lf, %lf>",
    euler_vector[0],
    euler_vector[1],
    euler_vector[2]);

  Serial.println(print_str);
  delay(250);
}
