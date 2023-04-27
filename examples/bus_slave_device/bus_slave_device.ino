// Bus slave device

#define B0 D2
#define B1 D3
#define B2 D4
#define B3 D5
#define B4 D6
#define B5 D7
#define B6 D8

uint8_t bits[7];
uint8_t bit0 = 0;
uint8_t bit1 = 0;
uint8_t bit2 = 0;
uint8_t bit3 = 0;
uint8_t bit4 = 0;
uint8_t bit5 = 0;
uint8_t bit6 = 0;

char print_buffer[128];

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Starting");
  // set all the pins to be inputs for the bus
  pinMode(B0, INPUT);
  pinMode(B1, INPUT);
  pinMode(B2, INPUT);
  pinMode(B3, INPUT);
  pinMode(B4, INPUT);
  pinMode(B5, INPUT);
  pinMode(B6, INPUT);

  bits[0] = 0;
  bits[1] = 0;
  bits[2] = 0;
  bits[3] = 0;
  bits[4] = 0;
  bits[5] = 0;
  bits[6] = 0;
  delay(1000);
}

void loop() {
  // bits[0] = digitalRead(B0);
  // bits[1] = digitalRead(B1);
  // bits[2] = digitalRead(B2);
  // bits[3] = digitalRead(B3);
  // bits[4] = digitalRead(B4);
  // bits[5] = digitalRead(B5);
  // bits[6] = digitalRead(B6);

  bit0 = digitalRead(B0);
  bit1 = digitalRead(B1);
  bit2 = digitalRead(B2);
  bit3 = digitalRead(B3);
  bit4 = digitalRead(B4);
  bit5 = digitalRead(B5);
  bit6 = digitalRead(B6);

  // probably a better way to do this but its a tuesday night
  // and im tired :(
  // logic for bit 0
  if (bit0 == HIGH) {
    bits[0] = 1;
  } else {
    bits[0] = 0;
  }

  // logic for bit 1
  if (bit1 == HIGH) {
    bits[1] = 1;
  } else {
    bits[1] = 0;
  }

  // logic for bit 2
  if (bit2 == HIGH) {
    bits[2] = 1;
  } else {
    bits[2] = 0;
  }

  // logic for bit 3
  if (bit3 == HIGH) {
    bits[3] = 1;
  } else {
    bits[3] = 0;
  }

  // logic for bit 4
  if (bit4 == HIGH) {
    bits[4] = 1;
  } else {
    bits[4] = 0;
  }

  // logic for bit 5
  if (bit5 == HIGH) {
    bits[5] = 1;
  } else {
    bits[5] = 0;
  }

  // logic for bit 6
  if (bit6 == HIGH) {
    bits[6] = 1;
  } else {
    bits[6] = 0;
  }


  sprintf(print_buffer, "| %d | %d | %d | %d | %d | %d |", 
    bits[0],
    bits[1],
    bits[2],
    bits[3],
    bits[4],
    bits[5],
    bits[6]
  );

  // sprintf(print_buffer, "| %d | %d | %d | %d | %d | %d |", 
  //   bit0,
  //   bit1,
  //   bit2,
  //   bit3,
  //   bit4,
  //   bit5,
  //   bit6
  // );

  Serial.print("Bits: ");
  // Serial.println(bits[0]);
  Serial.println(print_buffer);
  delay(100);
}
