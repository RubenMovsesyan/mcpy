// Bus master device

#define B0 10
#define B1 26
#define B2 18
#define B3 19
#define B4 23
#define B5 5
#define B6 13

uint8_t count = 0;
char print_buffer[128];

void setup() {
  Serial.begin(115200);
  while(!Serial);
  // set all the pins to be inputs for the bus
  pinMode(B0, OUTPUT);
  pinMode(B1, OUTPUT);
  pinMode(B2, OUTPUT);
  pinMode(B3, OUTPUT);
  pinMode(B4, OUTPUT);
  pinMode(B5, OUTPUT);
  pinMode(B6, OUTPUT);
}

void loop() {
  delay(500);

  digitalWrite(B0, ((count & 0x1) >> 0) * 255);
  digitalWrite(B1, ((count & 0x2) >> 1) * 255);
  digitalWrite(B2, ((count & 0x4) >> 2) * 255);
  digitalWrite(B3, ((count & 0x8) >> 3) * 255);
  digitalWrite(B4, ((count & 0x10) >> 4) * 255);
  digitalWrite(B5, ((count & 0x20) >> 5) * 255);
  digitalWrite(B6, ((count & 0x40) >> 6) * 255);

  sprintf(print_buffer, " :: | %d | %d | %d | %d | %d | %d |",
    ((count & 0x1) >> 0),
    ((count & 0x2) >> 1),
    ((count & 0x4) >> 2),
    ((count & 0x8) >> 3),
    ((count & 0x10) >> 4),
    ((count & 0x20) >> 5),
    ((count & 0x40) >> 6),
    count
  );

  Serial.print("Bits: ");
  Serial.print(count, BIN);
  Serial.println(print_buffer);
  count++;
}
