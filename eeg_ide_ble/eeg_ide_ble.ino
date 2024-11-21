#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Define UUIDs for the service and characteristic
#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0" // Custom Service UUID
#define CHARACTERISTIC_UUID "abcdef01-1234-5678-1234-56789abcdef0" // Custom Characteristic UUID

// Define ADC pins for three filters (each filter has two output pins)
const int adcPin1 = 34; // Filter 1 - Positive input
const int adcPin2 = 35; // Filter 1 - Negative input
const int adcPin3 = 32; // Filter 2 - Positive input
const int adcPin4 = 33; // Filter 2 - Negative input
const int adcPin5 = 25; // Filter 3 - Positive input
const int adcPin6 = 26; // Filter 3 - Negative input

// Sampling parameters
const unsigned long SAMPLE_RATE = 128; // Hz
const unsigned long SAMPLE_INTERVAL = 1000000 / SAMPLE_RATE; // in microseconds

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Callback class to handle client connections
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Client connected");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Client disconnected");
      // Restart advertising to allow new connections
      pServer->getAdvertising()->start();
      Serial.println("Advertising restarted");
    }
};

void setup() {
  Serial.begin(9600); // Ensure this matches the Serial Monitor baud rate
  Serial.println("Initializing ESP32...");

  // Initialize ADC pins
  pinMode(adcPin1, INPUT);
  pinMode(adcPin2, INPUT);
  pinMode(adcPin3, INPUT); // Initialize additional ADC pins
  pinMode(adcPin4, INPUT);
  pinMode(adcPin5, INPUT);
  pinMode(adcPin6, INPUT);
  Serial.println("ADC pins initialized.");

  // Initialize BLE
  BLEDevice::init("ESP32_EEG"); // BLE device name
  Serial.println("BLE Device initialized.");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("BLE Server created.");

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  Serial.println("BLE Service created.");

  // Create BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  Serial.println("BLE Characteristic created.");

  // Add descriptor for Client Characteristic Configuration
  pCharacteristic->addDescriptor(new BLE2902());
  Serial.println("BLE Descriptor added.");

  // Start the service
  pService->start();
  Serial.println("BLE Service started.");

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // Functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE advertising started, waiting for clients...");
}

void loop() {
  // Always read and print ADC values
  // Read ADC values for Filter 1
  int val1 = analogRead(adcPin1);
  int val2 = analogRead(adcPin2);
  int differential1 = val1 - val2;

  // Read ADC values for Filter 2
  int val3 = analogRead(adcPin3);
  int val4 = analogRead(adcPin4);
  int differential2 = val3 - val4;

  // Read ADC values for Filter 3
  int val5 = analogRead(adcPin5);
  int val6 = analogRead(adcPin6);
  int differential3 = val5 - val6;

  // Combine the differential values into a single string, separated by commas
  String dataStr = String(differential1) + "," + String(differential2) + "," + String(differential3);

  // Print ADC values to Serial Monitor for debugging
  Serial.println(dataStr);

  // Optionally, send BLE notifications if connected
  if (deviceConnected) {
    pCharacteristic->setValue(dataStr.c_str());
    pCharacteristic->notify();
  }

  // Wait for the next sample
  delayMicroseconds(SAMPLE_INTERVAL);
}
