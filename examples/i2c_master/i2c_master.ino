// I2C master device driver

#include <Wire.h>

#define SLAVE_ADDRESS 0x42

void setup() {
  Serial.begin(115200);
  Wire.begin(); // start I2C communication
}

void loop() {
  Wire.beginTransmission(SLAVE_ADDRESS); // Begin communication with the I2C slave device
  Wire.write("Hello, World!"); // Send data to the slave device
  Wire.endTransmission();

  delay(1000);
}
