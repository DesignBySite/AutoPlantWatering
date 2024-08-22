#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <time.h>
#include <Wire.h>
#include <string>
#include <numeric>
#include "./secrets.h"

Adafruit_ADS1015 ads;
WiFiClient wifiClient;
HTTPClient http;
WiFiUDP ntpUDP;
ESP8266WebServer server(80);
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char* ssid = SECRET_WIFI;
const char* password = SECRET_PASS;
const char* serverUrl = SECRET_SERVER;
unsigned long lastWateringTime = 0;
unsigned long lastServerCheckTime = 0;
const unsigned long serverCheckInterval = 300000; // 5 minutes
unsigned long startTime = 0;
int safetyTimer = 0;
int channelAddBy = 0;

void saveSensorData();
void wifiConnect();
void engageWateringProtocols();
void engageWateringProtocol(int channel, int pinNum);
void initiateWatering(int channel, int pinNum);
void engageWatering(int channel, int pinNum);
void disengageWatering(int channel, int pinNum);
int updateAndSendMoisture(int channel, const char* state, bool safetyFlag);
int readUntilValid(int channel);
int calculateMoisturePercentage(int adcValue);
void sendData(int channel, const char* message = nullptr);
void updateSensor(int channel, int moisture, const char* state, bool safetyFlag);
void getTime(char* buffer, size_t bufferSize);
void clearEEPROM();

#define MAX_DATE_LENGTH 20
#define MAX_STATE_LENGTH 50

struct SensorData {
    int sensorNumber;
    int moisture;
    char date[MAX_DATE_LENGTH];
    char state[MAX_STATE_LENGTH];
};

SensorData sensors[4];

void handlePost() {
    // Check if the request contains body data
    if (!server.hasArg("plain")) {
        server.send(500, "text/plain", "Server Error: Missing Data");
        return;
    }

    // Using a fixed buffer to hold the incoming JSON payload
    const int BUFFER_SIZE = 1024;
    char jsonBuffer[BUFFER_SIZE];

    // Read the incoming request into the buffer
    server.arg("plain").toCharArray(jsonBuffer, BUFFER_SIZE);

    // Parse the JSON object from the buffer
    StaticJsonDocument<BUFFER_SIZE> doc;
    DeserializationError error = deserializeJson(doc, jsonBuffer);
    
    if (error) {
        server.send(500, "text/plain", "Error parsing JSON");
        return;
    }

    // Extract data from JSON document
    int sensorNumber = doc["sensorNumber"]; // Assuming sensorNumber is always correctly provided

    // Perform bounds checking on sensorNumber if necessary
    if (sensorNumber < 0 || sensorNumber >= 4) {
        server.send(500, "text/plain", "Invalid sensor number");
        return;
    }

    // Set the sensor data from the JSON document
    saveSensorData();

    // Send response back to client
    server.send(200, "application/json", "{\"status\":\"received\"}");
}

void saveSensorData() {
    int address = 0;
    for (int i = 0; i < 4; i++) {
        EEPROM.put(address, sensors[i]);
        address += sizeof(sensors[i]);
    }
    EEPROM.commit();
}

void loadSensorData() {
    Serial.println("Loading Sensor Data");
    int address = 0;
    for (int i = 0; i < 4; i++) {
        EEPROM.get(address, sensors[i]);
        address += sizeof(sensors[i]);
    }
}

void adsBegin() {
  unsigned long startTime = millis();  // Record the start time
  unsigned long timeout = 10000;  // Set a timeout of 10000 milliseconds (10 seconds)
  
  Serial.println("Starting ADS initialization...");
    delay(1000);
  // Attempt to initialize the ADS repeatedly until the timeout or initialization is successful
  while (!ads.begin(0x48)) {
    Serial.println("Failed to initialize ADS device, retrying...");
    delay(500);  // Delay between retries to prevent flooding the I2C bus

    if (millis() - startTime > timeout) {
      Serial.println("Failed to initialize ADS device after 10 seconds.");
      break;  // Break the loop if the timeout is exceeded
    }
  }

  if (ads.begin(0x48)) {
    Serial.println("ADS device initialized successfully.");
    wifiConnect();
  }
}

void clearEEPROM() {
    for (int i = 0; i < 512; ++i) {  // Assume 512 bytes, adjust according to your EEPROM size
        EEPROM.write(i, 0);
    }
    EEPROM.commit();  // Make sure to commit the changes to EEPROM
}


void wifiConnect() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.print("Connected Successfully to ");
    Serial.println(SECRET_WIFI);
}

void setup() {
    Wire.begin(D2, D1); // SDA, SCL on ESP8266
    Serial.begin(9600);
    delay(500);
    adsBegin();
    delay(2000);
    

    Serial.println("Initializing further components...");
    EEPROM.begin(512);
    clearEEPROM();

    server.on("/updateFlag", HTTP_POST, handlePost);
    server.begin();
    

    pinMode(D3, OUTPUT);
    pinMode(D4, OUTPUT);
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);

    digitalWrite(D3, HIGH);
    digitalWrite(D4, HIGH);
    digitalWrite(D5, HIGH);
    digitalWrite(D6, HIGH);

    // Time configuration
    configTime(0, 0, "pool.ntp.org"); // Set timezone to UTC and NTP server
    timeClient.begin();  // Initialize the NTP client
    timeClient.update(); // Make an initial update

    // Wait for time synchronization
    while (!time(nullptr)) {
        Serial.println("Waiting for time...");
        delay(1000);
    }
    Serial.println("Done");
}


void loop() {

    if (millis() - lastServerCheckTime >= serverCheckInterval) {
        server.handleClient();
        lastServerCheckTime = millis();
    }
    
    if (millis() - lastWateringTime >= 1800000) { // 1 hour = 3,600,000ms 24 hours is 86400000
        sendData(0, "30 Minutes has elapsed");
        server.handleClient();
        loadSensorData();
        Serial.println("Data Loaded");
        engageWateringProtocols();
        lastWateringTime = millis();
        saveSensorData();
      // Get new reading from pin
    }
}
void engageWateringProtocols() {
    int channels[] = {D3, D4, D5, D6}; // Array of pin numbers for each channel
    int numChannels = sizeof(channels) / sizeof(channels[0]); // Calculate the number of channels

    for (int i = 0; i < numChannels; i++) {
        engageWateringProtocol(i, channels[i]); // Engage watering protocol for the current channel
        delay(2000); // Delay for 2 seconds before engaging the next channel
    }
}

void engageWateringProtocol(int channel, int pinNum) {
    int channelAdd = channel + channelAddBy;
    updateAndSendMoisture(channel, "off", false);
    Serial.println("Engaging Water Protocols");
    Serial.print("Channel: ");
    sendData(channelAdd, "Watering Protocol Entered");
    initiateWatering(channel, pinNum);
}

const unsigned long WATERING_TIME_MS = 60000;  // Time to water in milliseconds
const int MOISTURE_THRESHOLD = 10;             // Moisture level threshold
void initiateWatering(int channel, int pinNum) {
    int moisturePercentage = updateAndSendMoisture(channel, "off", false);
    Serial.print("moisture percent line 243: ");
    Serial.println(moisturePercentage);
    int sumChannel = channel + channelAddBy;
    sendData(sumChannel, "Inside Initiate Watering");

    if (moisturePercentage > MOISTURE_THRESHOLD) {
        sendData(sumChannel, "moisture above 10%, not watering");
        return;
    };

    engageWatering(channel, pinNum);
    disengageWatering(channel, pinNum);
}

void engageWatering(int channel, int pinNum) {
    unsigned long startTime = millis();
    bool dataSent = false;
    int sumChannel = channel + channelAddBy;
    updateAndSendMoisture(channel, "on", false);
    sendData(sumChannel, "Beginning to water");
    digitalWrite(pinNum, LOW); // Engage relay

    while (millis() - startTime <= WATERING_TIME_MS) {
        if (!dataSent) {
            sendData(sumChannel, "Inside while loop and actively watering");
            dataSent = true;
        }
        updateAndSendMoisture(channel, "on", false);
        delay(1000);  // Maintain a delay to avoid a busy loop
    }
}

void disengageWatering(int channel, int pinNum) {
    int sumChannel = channel + channelAddBy;
    updateAndSendMoisture(channel, "off", false);
    digitalWrite(pinNum, HIGH); // Disengage relay
    sendData(sumChannel, "Outside while loop, no longer watering");
}

int updateAndSendMoisture(int channel, const char* state, bool safetyFlag) {
    Serial.print("Channel: ");
    Serial.println(channel);
    yield();  // Allow other processes to run, useful in multitasking environments

    int adcValue = ads.readADC_SingleEnded(channel);  // Read ADC value once
    if (adcValue < 100) {  // Ensure ADC value is above threshold
        adcValue = readUntilValid(channel);
    }

    // Debug ADC value based on state
    if (strcmp(state, "on") == 0 || strcmp(state, "off") == 0) {
        Serial.print("ADCVALUE: ");
        Serial.println(adcValue);
        if (strcmp(state, "on") == 0) {
            Serial.println("State is on");
        }
    }

    yield();  // Another yield to maintain system responsiveness
    int moisturePercentage = calculateMoisturePercentage(adcValue);
    delay(100);  // Short delay for system stability

    Serial.print("Moisture: ");
    Serial.print(moisturePercentage);
    Serial.println("%");
    updateSensor(channel, moisturePercentage, state, safetyFlag);
    Serial.println("Sensor Updated");

    return moisturePercentage;
}

int readUntilValid(int channel) {
    int adcValue;
    do {
        adcValue = ads.readADC_SingleEnded(channel);
    } while (adcValue < 100);
    return adcValue;
}

int calculateMoisturePercentage(int adcValue) {
    return map(adcValue, 438, 938, 100, 0) - 20;
}


void sendJsonData(DynamicJsonDocument& doc) {
    const size_t bufferSize = 1024;  // Adjust size according to your expected JSON size
    char jsonBuffer[bufferSize];

    // Serialize JSON directly into the character buffer
    size_t bytesWritten = serializeJson(doc, jsonBuffer, bufferSize);
    if (bytesWritten == 0) {
        Serial.println("Failed to serialize JSON");
        return;
    }

    // Begin the HTTP request
    http.begin(wifiClient, serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    // Send the HTTP POST request
    int httpResponseCode = http.POST(jsonBuffer);
    if (httpResponseCode != 200) {
        Serial.print("HTTP Request failed: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}


void updateSensor(int channel, int moisture, const char* state, bool safetyFlag) {
    Serial.println("Update Sensor");
    sensors[channel].sensorNumber = channel;
    sensors[channel].moisture = moisture;
    getTime(sensors[channel].date, MAX_DATE_LENGTH);
    strncpy(sensors[channel].state, state, MAX_STATE_LENGTH);
    sensors[channel].state[MAX_STATE_LENGTH - 1] = '\0';  // Ensure null termination
}


void sendData(int channel, const char* message) {
    DynamicJsonDocument doc(1024);
    JsonObject data = doc.createNestedObject("data");
    int newChannel = channel - channelAddBy;
    data["message"] = message;
    data["sensorNumber"] = channel;
    data["moisture"] = sensors[newChannel].moisture;
    data["date"] = sensors[newChannel].date;
    data["state"] = sensors[newChannel].state;

  sendJsonData(doc);  // Assuming this function sends the JSON to your server
}


void getTime(char* buffer, size_t bufferSize) {
    time_t now = time(nullptr);  // Get current time
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, bufferSize, "%m-%d-%Y", timeinfo);
}