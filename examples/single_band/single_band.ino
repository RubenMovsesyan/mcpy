// This sketch demonstrates error feedback on a single band
// and assumes LEDs are used instead of vibration motors.
// No Bluetooth connectivity or other devices are required
// for this sketch to work.

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// ===== Developer definitions =====
#define FORCE_SERIAL 1
#define DEBUG_PRINT_DIRECTION 0
#define DEBUG_PRINT_VECTORS 1
#define DEBUG_PRINT_CALIBRATION 1

// ===== Hardware definitions =====
// FOR THE RED AND BLUE BAND
// #define UP_MOTOR        D9
// #define DOWN_MOTOR      D7
// #define LEFT_MOTOR      D6
// #define RIGHT_MOTOR     D8

// FOR THE BLUE AND YELLOW BAND
#define UP_MOTOR        D9
#define DOWN_MOTOR      D6
#define LEFT_MOTOR      D8
#define RIGHT_MOTOR     D7

#define MAX_LED 255
#define BASE_LED 255

// ===== Other definitions =====
#define SAMPLE_PERIOD_MS 10
#define GRACE_ANGLE_DEGREES 15

//  ===== Global variables =====
char print_string[64];
Adafruit_BNO055 bno;
imu::Vector<3> correct_vector, error_vector, raw_vector, calibrate_vector, curr_vector;
// correct_vector is where we should be.
// raw_vector is the pre-processed reading from the device.
// calibrate_vector is the reading at the time of calibration and
//    all processed readings are relative to this.
// curr_vector is the post-processed orientation of the current value.

uint8_t feedback[3] = {0, 0, 0};
// feedback based on error *per axis*.
// E.g. motor or LED intensity on the pitch, yaw, or roll axis.

void printVector(imu::Vector<3> &vec, bool newline = false) {
  sprintf(print_string, "vector: <%f, %f, %f>", vec[0], vec[1], vec[2]);
  Serial.print(print_string);
  if (newline) Serial.println();
}

bool isMostlyCalibrated(Adafruit_BNO055 &bno, uint8_t *system, uint8_t *gyro, uint8_t *accel, uint8_t *mag) {
  bno.getCalibration(system, gyro, accel, mag);
  if (DEBUG_PRINT_CALIBRATION) {
    sprintf(print_string, "calibration: %d, %d, %d, %d", *system, *gyro, *accel, *mag);
    Serial.println(print_string);
  }
  return (*gyro >= 1 && *accel >= 1 && *mag >= 2);
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

void initSerial() {
  Serial.begin(9600);
  if (FORCE_SERIAL) while (!Serial);
  else delay(2000);
  Serial.println("Serial communications initialized.");
}

void initHardware() {
  if (!bno.begin(OPERATION_MODE_NDOF)) {
    Serial.println("\nFailed to find BNO055 chip");
    while (1);
  }
  Serial.println("\nBNO055 Found!");
  bno.enterNormalMode();

  analogWriteResolution(8);
  pinMode(UP_MOTOR,     OUTPUT);
  pinMode(DOWN_MOTOR,   OUTPUT);
  pinMode(LEFT_MOTOR,   OUTPUT);
  pinMode(RIGHT_MOTOR,  OUTPUT);
}

void updateHardware() {
  raw_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  curr_vector = raw_vector - calibrate_vector;
  curr_vector = normalizeEulerVector(curr_vector);
  error_vector = curr_vector - correct_vector;
  // Don't normalize error_vector probably?
  // It will be within <-360 to +360, N/A, -360 to +360> range so idk if that's bad or not.
  error_vector = normalizeEulerVector(error_vector);

  if (DEBUG_PRINT_VECTORS) {
    Serial.print("Raw ");
    printVector(raw_vector, false);
    Serial.print(", Curr ");
    printVector(curr_vector, false);
    Serial.print(", Error ");
    printVector(error_vector, true);
  }
  // should we normalize these vectors to be within -180 to +180, etc?
  // or does the subtraction operator already do it for us?

  // Calculate the feedback intensity in each axis.
  feedback[0] = max(0.0, min((uint8_t)(fabs(error_vector[0])) - GRACE_ANGLE_DEGREES + BASE_LED, MAX_LED));
  // feedback[1] = 0; // roll is currently unused.
  feedback[2] = max(0.0, min((uint8_t)(2 * fabs(error_vector[2])) - GRACE_ANGLE_DEGREES + BASE_LED, MAX_LED));

  if (error_vector[0] >= GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Right, ");
    analogWrite(RIGHT_MOTOR, feedback[0]);
    analogWrite(LEFT_MOTOR, 0);
  } else if (error_vector[0] <= -GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Left, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, feedback[0]);
  } else {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Grace, ");
    analogWrite(RIGHT_MOTOR, 0);
    analogWrite(LEFT_MOTOR, 0);
  }

  if (error_vector[2] >= GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Down, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, feedback[2]);
  } else if (error_vector[2] <= -GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Up, ");
    analogWrite(UP_MOTOR, feedback[2]);
    analogWrite(DOWN_MOTOR, 0);
  } else {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Grace, ");
    analogWrite(UP_MOTOR, 0);
    analogWrite(DOWN_MOTOR, 0);
  }
  // Serial.println();

  delay(SAMPLE_PERIOD_MS);
}

// calibration debug variables (returns 0 thru 3 to indicate how calibrated each device is).
uint8_t system_cal, gyro_cal, accel_cal, mag_cal;

void setup() {
  // This is a POST-PROCESS vector! <0, 0, 0> means curr_vector == calibrate_vector!
  correct_vector = {0, 0, 0};
  initSerial();
  initHardware();
  Serial.println("Calibrating BNO055, please extend your arms and *slowly* complete a full spin.");
  while (!isMostlyCalibrated(bno, &system_cal, &gyro_cal, &accel_cal, &mag_cal));
  Serial.println("Capturing reference position in 5 seconds...");
  delay(5000);
  calibrate_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  Serial.println("Calibrated.");
}

void loop() {
  updateHardware();
}
