// This sketch demonstrates error feedback on a single band
// and assumes LEDs are used instead of vibration motors.
// No Bluetooth connectivity or other devices are required
// for this sketch to work.

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// ===== Developer definitions =====
#define FORCE_SERIAL 1

// ===== Other definitions =====
#define SAMPLE_PERIOD_MS 10

//  ===== Global variables =====
char print_string[64];
Adafruit_BNO055 bno;
imu::Vector<3> raw_vector, calibrate_vector;

void printVector(imu::Vector<3> &vec, bool newline = false) {
  sprintf(print_string, "vector: <%f, %f, %f>", vec[0], vec[1], vec[2]);
  Serial.print(print_string);
  if (newline) Serial.println();
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
}

void setup() {
  initSerial();
  initHardware();
  Serial.println("Calibrating BNO055 in 5 seconds...");
  delay(5000);
  calibrate_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  Serial.println("Calibrated.");
}

void loop() {
  raw_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  Serial.print("Raw ");
  printVector(raw_vector, true);

  delay(SAMPLE_PERIOD_MS);
}
