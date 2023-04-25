// BLE includes
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// WiFi includes
#include <WiFi.h>
#include <WiFiUdp.h>

// Other includes
// #include <Wire.h>

#define SSID "ESP32"
#define PASS "password"

// BLE characteristics
#define SERVICE_UUID            "e9eaebf7-baec-46ee-aac7-3633b12a048e"
#define CHARACTERISTIC_UUID     "24ba02e5-59cf-4227-94f3-0bc646fa6362"

// packet size from periperhals
// can be changed later
#define PACKET_SIZE 256

#define UDP_PORT 8080

// BLE globals
BLEServer*                      pServer = NULL;
BLECharacteristic*              pCharacteristic = NULL;
bool                            deviceConnected = false;
bool                            oldDeviceConnected = false;
std::string                     value;

// BLE Callback functions
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    // Serial.println("Connected to BLE device");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    // Serial.println("Disconnected from BLE device");
  }
};

// WiFi globals
WiFiUDP udp;

void setupWiFi() {
  // Set up wifi
  WiFi.softAP(SSID, PASS);
  // Serial.println("Access point started");
  // Serial.print("SSID: ");
  // Serial.println(SSID);
  // Serial.print("IP address: ");
  // Serial.println(WiFi.softAPIP());

  // Set up udp port
  udp.begin(UDP_PORT);
  // Serial.print("Listening for UDP broadcasts on port ");
  // Serial.println(UDP_PORT);
}

void setupBLE() {
  // Create BLE device
  BLEDevice::init("ESP_BLE");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ      |
    BLECharacteristic::PROPERTY_WRITE     |
    BLECharacteristic::PROPERTY_NOTIFY    |
    BLECharacteristic::PROPERTY_INDICATE
  );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  // Serial.println("Waiting a client connection to notify...");
}

void setup() {
  // Serial.begin(115200);
  // while (!Serial);
  
  setupWiFi();
  setupBLE();
}

void loop() {
  // get udp packet from peripherals
  char buffer[PACKET_SIZE];
  int packet_size = udp.parsePacket();
  if (packet_size) {

    udp.read(buffer, PACKET_SIZE);

    // Serial.println(buffer);
    
    // euler_vector_string.writeValue(buffer);
    value = buffer;
  }

  // notify changed value
  if (deviceConnected) {
    pCharacteristic->setValue((uint8_t*)(value.c_str()), PACKET_SIZE);
    pCharacteristic->notify();
    // value[0] = (value[0] + 1) % 126;
    // if (value[0] < 33) value[0] = 33;
    // Serial.println(value.c_str());
    delay(3);
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}
