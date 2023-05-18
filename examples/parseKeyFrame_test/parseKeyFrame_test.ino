#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

void parseKeyFrame(byte buf[24], imu::Vector<3> &joint_vec, imu::Vector<3> &diff_vec) {
  float j1 = 0, j2 = 0, j3 = 0;
  float d1 = 0, d2 = 0, d3 = 0;

  memcpy((void*)&j1, &buf[0], sizeof(float));
  memcpy((void*)&j2, &buf[4], sizeof(float));
  memcpy((void*)&j3, &buf[8], sizeof(float));

  memcpy((void*)&d1, &buf[12], sizeof(float));
  memcpy((void*)&d2, &buf[16], sizeof(float));
  memcpy((void*)&d3, &buf[20], sizeof(float));

  joint_vec[0] = j1;
  joint_vec[1] = j2;
  joint_vec[2] = j3;

  diff_vec[0] = d1;
  diff_vec[1] = d2;
  diff_vec[2] = d3;
}

void printVector(char* print_string, imu::Vector<3> vec, bool newline = false) {
  sprintf(print_string, "vector: <%f, %f, %f>", vec[0], vec[1], vec[2]);
  Serial.print(print_string);
  if (newline) Serial.println();
}

void reverseBytes(byte* buf, int interval, int interval_count) {
  byte temps[interval];
  for (int i = 0; i < interval_count; i++) {
    for (int j = (i * interval); j < ((i + 1) * interval); j++) {
      temps[j] = buf[j];
    }
    for (int j = (i * interval); j < ((i + 1) * interval); j++) {
      buf[j] = temps[((i + 1) * interval) - 1 - j];
    }
  }
}

char print_string[64];
// 0x40 0xa0 0x00 0x00,
byte buf[24] = {0x40, 0xa0, 0x00, 0x00, 0x40, 0xa0, 0x00, 0x00, 0x40, 0xa0, 0x00, 0x00, 0x40, 0xa0, 0x00, 0x00, 0x40, 0xa0, 0x00, 0x00, 0xc0, 0xa0, 0x00, 0x00};
// byte buf[24] = {0x00, 0x00, 0xa0, 0x40, 0x00, 0x00, 0xa0, 0x40, 0x00, 0x00, 0xa0, 0xc0, 0x00, 0x00, 0xa0, 0xc0, 0x00, 0x00, 0xa0, 0xc0, 0x00, 0x00, 0xa0, 0xc0};
imu::Vector<3> joint_vec, diff_vec;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Serial initalized.");

  Serial.println(sizeof(float));

  parseKeyFrame(buf, joint_vec, diff_vec);
  printVector(print_string, joint_vec, true);
  printVector(print_string, diff_vec, true);

  for (int i = 0; i < 24; i++) {
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  reverseBytes(buf, sizeof(float), 6);

  for (int i = 0; i < 24; i++) {
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  parseKeyFrame(buf, joint_vec, diff_vec);
  printVector(print_string, joint_vec, true);
  printVector(print_string, diff_vec, true);
  // buf[0] = 0xc0;
  // buf[1] = 0xa0;
  // buf[2] = 0x00;
  // buf[3] = 0x00;
  // buf[4] = 0x00000000;
  // buf[5] = 0x40400000;
  // Serial.print(buf[0], BIN);
  // Serial.print(" ");
  // Serial.print(buf[1], BIN);
  // Serial.print(" ");
  // Serial.println();
  // // parseKeyFrame(buf, &joint_vec, &diff_vec);
  // float KILL_ME_PLEASE = 0.0;
  // memcpy((void*)&KILL_ME_PLEASE, &buf[0], sizeof(float));
  // Serial.print("Kill: ");
  // Serial.println(KILL_ME_PLEASE);
  // joint_vec[0] = KILL_ME_PLEASE;
  // memcpy(&joint_vec[1], &buf[4], sizeof(float));
  // memcpy(&joint_vec[2], &buf[8], sizeof(float));
  // Serial.print("Kill 2: ");
  // Serial.println(KILL_ME_PLEASE);
  // Serial.println(joint_vec[0]);

  // memcpy(&diff_vec[0], &buf[12], sizeof(float));
  // memcpy(&diff_vec[1], &buf[16], sizeof(float));
  // memcpy(&diff_vec[2], &buf[20], sizeof(float));

  // printVector(print_string, joint_vec, true);
  // printVector(print_string, diff_vec, true);
}

void loop() {
  // put your main code here, to run repeatedly:

}
