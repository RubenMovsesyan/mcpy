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

// Given an Euler vector with unbounded dimensions in any of the
// yaw, pitch, roll axes, return a vector with the equivalent
// angles within the ranges of <-180 to +180, -90 to +90, -180 to +180>
// respectively. (BNO055 <yaw, roll, pitch> are in range of
// <0 to +360, -90 to +90, -180 to +180> respectively so only the
// yaw is remapped to a entirely different range.)
// NOTE: This funciton truncates all floats :)
imu::Vector<3> normalizeEulerVector(imu::Vector<3> &vec) {
  imu::Vector<3> result;
  int roll, pitch;
  result[0] = (int)vec[0] % 360; // get yaw within 0 to 360
  // result[0] -= 180.0; // get yaw within -180 to +180.
  roll = (int)result[1];
  if (roll >= 0) roll = roll % 90;
  else roll = -1 * (abs(roll) % 90);
  result[1] = (float)roll;
  pitch = (int)result[2];
  if (pitch >= 0) pitch = pitch % 180;
  else pitch = -1 * (abs(pitch) % 180);
  result[2] = (float)pitch;

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
  error_vector = curr_vector - correct_vector;
  // curr_vector = normalizeEulerVector(curr_vector);
  // error_vector = normalizeEulerVector(error_vector);
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

void setup() {
  // This is a POST-PROCESS vector! <0, 0, 0> means curr_vector == calibrate_vector!
  correct_vector = {0, 0, 0};
  initSerial();
  initHardware();
  Serial.println("Calibrating BNO055 in 5 seconds...");
  delay(5000);
  calibrate_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  Serial.println("Calibrated.");
}

void loop() {
  updateHardware();
}
