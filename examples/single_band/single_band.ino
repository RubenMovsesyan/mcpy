// This sketch demonstrates error feedback on a single band
// and assumes LEDs are used instead of vibration motors.
// No Bluetooth connectivity or other devices are required
// for this sketch to work.

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <mocopy.h>

using namespace mocopy;

// For the blue and yellow band.
#define UP_MOTOR        D9
#define DOWN_MOTOR      D6
#define LEFT_MOTOR      D8
#define RIGHT_MOTOR     D7

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

void updateHardware() {
  raw_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  curr_vector = raw_vector - calibrate_vector;
  curr_vector = normalizeEulerVector(curr_vector);
  error_vector = curr_vector - correct_vector;
  error_vector = normalizeEulerVector(error_vector);

  if (DEBUG_PRINT_VECTORS) {
    Serial.print("Raw ");
    printVector(print_string, raw_vector, false);
    Serial.print(", Curr ");
    printVector(print_string, curr_vector, false);
    Serial.print(", Error ");
    printVector(print_string, error_vector, true);
  }

  // Calculate the feedback intensity in each axis.
  feedback[0] = max(0.0, min((uint8_t)(fabs(error_vector[0])) - GRACE_ANGLE_DEGREES + BASE_LED, MAX_LED));
  // feedback[1] = 0; // roll is currently unused.
  feedback[2] = max(0.0, min((uint8_t)(2 * fabs(error_vector[2])) - GRACE_ANGLE_DEGREES + BASE_LED, MAX_LED));

  if (error_vector[0] >= GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Right, ");
    analogWrite(DOWN_MOTOR, feedback[0]);
    analogWrite(LEFT_MOTOR, 0);
  } else if (error_vector[0] <= -GRACE_ANGLE_DEGREES) {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Left, ");
    analogWrite(DOWN_MOTOR, 0);
    analogWrite(LEFT_MOTOR, feedback[0]);
  } else {
    if (DEBUG_PRINT_DIRECTION) Serial.print("Grace, ");
    analogWrite(DOWN_MOTOR, 0);
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
  initHardware(bno, UP_MOTOR, DOWN_MOTOR, LEFT_MOTOR, RIGHT_MOTOR);
  calibrateBNO(bno, calibrate_vector);
}

void loop() {
  updateHardware();
}
