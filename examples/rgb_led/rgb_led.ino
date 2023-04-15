#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <string>

#define RED_PIN 16
#define GREEN_PIN 0
#define BLUE_PIN 14

// Global Variables
char print_str[100];

void setup() {
  Serial.begin(115200);
  Serial.println("\nSerial communication initialized.");
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(RED_PIN, HIGH);
  delay(500);
  digitalWrite(RED_PIN, LOW);
  delay(500);
  digitalWrite(GREEN_PIN, HIGH);
  delay(500);
  digitalWrite(GREEN_PIN, LOW);
  delay(500);
  digitalWrite(BLUE_PIN, HIGH);
  delay(500);
  digitalWrite(BLUE_PIN, LOW);
  delay(500);
}
