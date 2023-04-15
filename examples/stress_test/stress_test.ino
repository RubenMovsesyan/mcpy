#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <string>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define WIFI_SSID "ESP32"
#define WIFI_PASS "password"

#define UDP_PORT 8080

#define RED D0
#define GREEN D3
#define BLUE TX

#define M1 D5
#define M2 D6
#define M3 D7
#define M4 D8

int count = 0;

// Global Variables
Adafruit_BNO055 bno;
WiFiUDP UDP;
IPAddress ip(255, 255, 255, 255);
imu::Vector<3> euler_vector;
sensors_event_t imu_event;
char print_str[512];

int currTime;
int prevTime;

void setup() {
  Serial.begin(115200);
  // Try to initialize the IMU
  if (!bno.begin()) {
		Serial.println("Failed to find BNO055 chip");
		while (1) {
		  delay(10);
		}
	}
	Serial.println("BNO055 Found!");
  bno.enterNormalMode();

  // Begin UDP
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  // Connected to WiFi
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Set up UDP
  UDP.begin(UDP_PORT);
  Serial.print("Broadcasting UDP port ");
  Serial.println(UDP_PORT);

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  
  analogWriteResolution(8);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(M3, OUTPUT);
  pinMode(M4, OUTPUT);

  prevTime = millis();
}

void loop() {
  currTime = millis();
  while (currTime < prevTime + 10) {
    currTime = millis();
  }

  euler_vector = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  sprintf(
    print_str,
    "Euler Vector <%f, %f, %f>\n",
    euler_vector[0],
    euler_vector[1],
    euler_vector[2]);

  // Serial.print(print_str);
  
  prevTime = millis();

  UDP.beginPacket(ip, UDP_PORT);
  UDP.write(print_str);
  UDP.endPacket();

  // if (count >= 0 && count < 500) {
  //   digitalWrite(RED, HIGH);
  //   digitalWrite(GREEN, LOW);
  //   digitalWrite(BLUE, LOW);
  // } else if (count >= 500 && count < 1000) {
  //   digitalWrite(RED, LOW);
  //   digitalWrite(GREEN, HIGH);
  //   digitalWrite(BLUE, LOW);
  // } else if (count >= 1000 && count < 1500) {
  //   digitalWrite(RED, LOW);
  //   digitalWrite(GREEN, LOW);
  //   digitalWrite(BLUE, HIGH);
  // } else {
  //   count = 0;
  // }
  // count++;
  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, HIGH);
  digitalWrite(BLUE, HIGH);

  analogWrite(M1, 255);
  analogWrite(M2, 255);
  analogWrite(M3, 255);
  analogWrite(M4, 255);
}