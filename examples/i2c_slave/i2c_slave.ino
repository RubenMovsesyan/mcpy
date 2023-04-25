// I2C slave device driver

#include <Wire.h>

#define SLAVE_ADDRESS 0x42

void setup() {
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  Serial.begin(115200);
  while (!Serial);
}

void loop() {

}

void receiveData(int byteCount) {
  // individually reads each byte and prints to console
  while (Wire.available()) {
    char data = Wire.read(); // Read incoming data from I2C bus
    Serial.print("Received Data: ");
    Serial.println(data);
  }
}
