#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define BNO_RESET D2
#define STR_SIZE 64

Adafruit_BNO055 bno;
imu::Vector<3> joint_vector;

char print_string[STR_SIZE];

int timer = 1000;

void setup() {
  Serial.begin(9600);
  delay(2000);

  pinMode(BNO_RESET, OUTPUT);
  digitalWrite(BNO_RESET, HIGH);

  // start the bno
  if (!bno.begin(OPERATION_MODE_NDOF)) {
    Serial.println("\nFailed to find BNO055 chip");
    while (1);
  }

  Serial.println("\nBNO055 Found!");
  bno.enterNormalMode();
}

void loop() {
  joint_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  sprintf(print_string, 
    "EuV <%f, %f, %f>",
    joint_vector[0],
    joint_vector[1],
    joint_vector[2]
  );

  Serial.println(print_string);

  timer--;

  if (timer == 0) {
    Serial.println("Resetting timer...");
    timer = 1000;
    digitalWrite(BNO_RESET, LOW);
    delayMicroseconds(1);
    digitalWrite(BNO_RESET, HIGH);
    delay(800);

    bno.enterNormalMode();
  }

  // if (timer == 99) {
  //   digitalWrite(BNO_RESET, LOW);
  // }

  delay(10);
}
