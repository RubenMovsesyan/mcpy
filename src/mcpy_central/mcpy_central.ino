// BLE central device that creates a server to connect to the phone and
// Scans for a accelerometer service

#include <ArduinoBLE.h>

// --------------------------- BLE defines -------------------------

// Local UUIDs
#define CENTRAL_SERVICE_UUID "0c35e466-ad83-4651-88fa-0ff9d70fbf8c"
#define FORWARD_CHARACTERISTIC_UUID "a59d3afb-5010-43f0-a241-1ad27e92d7b9"

// UUIDs for joint device
#define JOINT_SERVICE_UUID "a9a95e92-26ea-4282-bd0c-7c8bd6c65a2b"
#define REP_COMPLETION_CHARACTERISTIC_UUID "08d54caf-75bc-4aa6-876b-8eea5427605a"

// --------------------------- BLE defines -------------------------

#define STR_SIZE 64

char print_string[STR_SIZE];

BLEDevice joint, app;
BLEService central_service(CENTRAL_SERVICE_UUID);
BLEFloatCharacteristic forward_characteristic(FORWARD_CHARACTERISTIC_UUID);

byte buf[4] = {0};
float diff = 0.0;

// This initializes the BLE server for the phone to connect to
void initBLE() {
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  Serial.println("Successfully initialized Bluetooth® Low Energy module.");

  // Construct the service to be advertised
  central_service.addCharacteristic(forward_characteristic);
  BLE.addService(central_service);

  // Setup central advertising
  BLE.setLocalName("MoCopy (central)");
  BLE.setAdvertisedService(central_service);
  BLE.advertise();

  Serial.print("Advertising with address: ");
  Serial.println(BLE.address().c_str());

  // Scan for joint service
  Serial.println("Scanning for Joint service");
  do {
    BLE.scanForUuid(JOINT_SERVICE_UUID);
    joint = BLE.available();
  } while (!joint);

  Serial.println("Joint device found.");
  Serial.print("Device MAC address: ");
  Serial.println(joint.address());
  Serial.print("Device Name: ");
  Serial.println(joint.localName());

  // Print all service advertised by the joint
  if (joint.hasAdvertisedServiceUuid()) {
    Serial.println("Joint's advertised services UUIDs: ");
    for (int i = 0; i < joint.advertisedServiceUuidCount(); i++) {
      Serial.print(joint.advertisedServiceUuid(i));
      Serial.print(", ");
    }
    Serial.println();
  } else {
    Serial.println("Joint has no advertised services!");
  }

  BLE.stopScan();
}

void connectToJoint() {
  Serial.println("Discovering joint device...");

  do {
    BLE.scanForUuid(JOINT_SERVICE_UUID);
    joint = BLE.available();
  } while (!joint);

  if (joint) {
    Serial.println("Joint device found!");
    Serial.print("* Device MAC address: ");
    Serial.println(joint.address());
    Serial.print("* Device Name: ");
    Serial.println(joint.localName());
    Serial.print("* Advertised service UUID: ");
    Serial.println(joint.advertisedServiceUuid());
    Serial.println();
    BLE.stopScan();
  }
}

void controlJoint() {
  Serial.println("- Connecting to joint device...");

  if (joint.connect()) {
    Serial.println("* Connect to joint device!");
    Serial.println();
  } else {
    Serial.println("* Connection to joint device failed!");
    Serial.println();
    return;
  }

  Serial.println("- Discovering joint device attributes...");
  if (joint.discoverAttributes()) {
    Serial.println("* joint device attributes discovered!");
    Serial.println();
  } else {
    Serial.println("* joint device attributes discovery failed!");
    Serial.println();
    joint.disconnect();
    return;
  }

  BLECharacteristic rep_completion_characteristic = joint.characteristic(REP_COMPLETION_CHARACTERISTIC_UUID);

  if (!rep_completion_characteristic) {
    Serial.println("* joint device does not have the expected characteristic!");
    joint.disconnect();
    return;
  } else if (!rep_completion_characteristic.canSubscribe()) {
    Serial.println("* Periheral device does not have a subscribable characteristic!");
    joint.disconnect();
    return;
  }

  rep_completion_characteristic.readValue(buf, 4);
  memcpy(&diff, buf, 4);

  Serial.print("diff: ");
  Serial.println(diff);
}

void updateBLE() {
  
}

// Arduino Functions \/ ------------------------------------------ \/
 
void setup() {
  Serial.begin(9600);
  while (!Serial);

  initBLE();
  // connectToJoint();
}

void loop() {
  controlJoint();
}
