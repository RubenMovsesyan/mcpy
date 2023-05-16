#ifndef MOCOPY_H
#define MOCOPY_H

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

namespace mocopy {

// ===== Developer definitions =====
#define FORCE_SERIAL 1
#define DEBUG_PRINT_DIRECTION 0
#define DEBUG_PRINT_VECTORS 1
#define DEBUG_PRINT_CALIBRATION 1

// ===== UUID definitions =====
// --- External UUIDs ---
#define EXTERNAL_SERVICE_UUID "56176a63-d563-43f4-b239-636f41b63c6d"
#define EXTERNAL_ORIENTATION_CHARACTERISTIC_UUID "04308b2c-90dc-4984-8c45-81650dff60b8"
#define RESET_BNO_EXTERNAL_CHARACTERISTIC_UUID "c162bd0b-e48d-42c2-86f6-45ef8f615929"
#define JOINT_ORIENTATION_CHARACTERISTIC_UUID "b99cc0f3-8cdc-4bb1-a51d-3927431f0985"
// --- Joint UUIDS ---
#define JOINT_SERVICE_UUID "a9a95e92-26ea-4282-bd0c-7c8bd6c65a2b"
#define RESET_BNO_JOINT_CHARACTERISTIC_UUID "356e9144-fd4f-4ad7-ad60-983f551e5c0c"
#define PITCH_DIFF_CHARACTERISTIC_UUID "3ffdaee3-9acf-42ad-abe5-b078671f26da"
// --- Central UUIDS ---

// (what is this? v)
#define REP_COMPLETION_CHARACTERISTIC_UUID "08d54caf-75bc-4aa6-876b-8eea5427605a"

// ===== Hardware definitions =====
#define MAX_LED 255
#define BASE_LED 20

// ===== Other definitions =====
#define SAMPLE_PERIOD_MS 10
#define GRACE_ANGLE_DEGREES 15

uint8_t min(uint8_t a, uint8_t b) {if (a <= b) return a; else return b; }
uint8_t max(uint8_t a, uint8_t b) {if (a >= b) return a; else return b; }

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

// Raw BNO vectors are within the following ranges:
// <0 to 360, -90 to +90, -180 to +180>
// We want to use processed vectors in these ranges:
// <-180 to +180, N/A, -180 to +180>
// NOTE: This truncates every float during int cast.
imu::Vector<3> normalizeEulerVector(imu::Vector<3> &vec) {
  imu::Vector<3> result;
  result[0] = (((int)vec[0] + 360) % 360) - 180;
  result[1] = vec[1];
  result[2] = (((int)vec[2] + 360) % 360) - 180;

  return result;
}

void initHardware(Adafruit_BNO055 &bno, int up, int down, int left, int right) {
  if (!bno.begin(OPERATION_MODE_NDOF)) {
    Serial.println("\nFailed to find BNO055 chip");
    while (1);
  }
  Serial.println("\nBNO055 Found!");
  bno.enterNormalMode();

  analogWriteResolution(8);
  pinMode(up,     OUTPUT);
  pinMode(down,   OUTPUT);
  pinMode(left,   OUTPUT);
  pinMode(right,  OUTPUT);
}

void initSerial() {
  Serial.begin(9600);
  if (FORCE_SERIAL) while (!Serial);
  else delay(2000);
  Serial.println("Serial communications initialized.");
}

}

#endif