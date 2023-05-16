#include "mocopy.h"

using namespace mocopy;

void printVector(char* print_string, imu::Vector<3> &vec, bool newline = false) {
  sprintf(print_string, "vector: <%f, %f, %f>", vec[0], vec[1], vec[2]);
  Serial.print(print_string);
  if (newline) Serial.println();
}

bool isMostlyCalibrated(Adafruit_BNO055 &bno, uint8_t *system, uint8_t *gyro, uint8_t *accel, uint8_t *mag, char* print_string) {
  bno.getCalibration(system, gyro, accel, mag);
  if (DEBUG_PRINT_CALIBRATION) {
    sprintf(print_string, "calibration: %d, %d, %d, %d", *system, *gyro, *accel, *mag);
    Serial.println(print_string);
  }
  return (*gyro >= 0 && *accel >= 1 && *mag >= 2);
}

void calibrateBNO(Adafruit_BNO055 &bno, imu::Vector<3> &calibrate_vector) {
  // calibration debug variables (returns 0 thru 3 to indicate how calibrated each device is).
  uint8_t system_cal, gyro_cal, accel_cal, mag_cal;
  char print_string[64];

  Serial.println("Calibrating BNO055, please wiggle your arms (no I'm not kidding).");
  while (!isMostlyCalibrated(bno, &system_cal, &gyro_cal, &accel_cal, &mag_cal, print_string));
  Serial.println("Capturing reference position in 5 seconds...");
  delay(5000);
  calibrate_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  Serial.println("Calibrated.");
}

imu::Vector<3> normalizeEulerVector(imu::Vector<3> &vec) {
  imu::Vector<3> result;
  result[0] = (((int)vec[0] + 360) % 360) - 180;
  result[1] = vec[1];
  result[2] = (((int)vec[2] + 360) % 360) - 180;

  return result;
}

void initSerial() {
  Serial.begin(9600);
  if (FORCE_SERIAL) while (!Serial);
  else delay(2000);
  Serial.println("Serial communications initialized.");
}