#define LED D2
#define BASE_VIBRATION 20

int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(LED, OUTPUT);
  analogWriteResolution(8);
  analogWrite(LED, 0);
}

void loop() {
  int percent = 0;
  Serial.println("For the greater good");
  analogWrite(LED, map(percent, 0, 100, BASE_VIBRATION, 255));
  delay(1000);
  analogWrite(LED, 0);
  delay(1000);
}
