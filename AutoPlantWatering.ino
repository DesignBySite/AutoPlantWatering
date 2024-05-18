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
const unsigned long wateringInterval = 3000000;  // 50 minutes
unsigned long startTime = 0;
int safetyTimer = 0;

void saveSensorData();
void wifiConnect();
void engageWateringProtocols();
void engageWateringProtocol(int channel, int pinNum);
void initiateWatering(int channel, int pinNum);
void safetyCheck(int channel, unsigned long startTime, unsigned long endTime);
int updateAndSendMoisture(int channel, const char* state, bool safetyFlag);
void sendData(int channel);
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
    bool safetyFlag;
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
    bool safetyFlag = doc["safetyFlag"]; // Assuming safetyFlag is always correctly provided

    // Perform bounds checking on sensorNumber if necessary
    if (sensorNumber < 0 || sensorNumber >= 4) {
        server.send(500, "text/plain", "Invalid sensor number");
        return;
    }

    // Set the sensor data from the JSON document
    sensors[sensorNumber].safetyFlag = safetyFlag;
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
}


void loop() {
    server.handleClient();
    loadSensorData();
    Serial.println("Data Loaded");
    if (millis() - lastWateringTime >= 3,600,000) { // 1 hour = 3,600,000ms
        engageWateringProtocols();
        lastWateringTime = millis();
    }
}
void engageWateringProtocols() {
    engageWateringProtocol(0, D3);
    delay(2000); // Delay for 2 seconds before next channel engages
    engageWateringProtocol(1, D4);
    delay(2000); // Repeat as needed
    engageWateringProtocol(2, D5);
    delay(2000);
    engageWateringProtocol(3, D6);
}

void engageWateringProtocol(int channel, int pinNum) {
    Serial.println("Engaging Water Protocols");
    Serial.print("Channel: ");
    Serial.println(channel);
  if (sensors[channel].safetyFlag) {
    Serial.print("Safety Flag for Channel: ");
    Serial.println(channel);
    digitalWrite(pinNum, HIGH); // Disengage relay
    return;
  }
  initiateWatering(channel, pinNum);
 
}


void initiateWatering(int channel, int pinNum) {
    Serial.println("Initiate Watering");
    int moisturePercentage = updateAndSendMoisture(channel, "off", false);
    unsigned long startTime = millis();

    if (moisturePercentage <= 10 && !sensors[channel].safetyFlag) {
        Serial.println("Watering");
        digitalWrite(pinNum, LOW); // Engage relay
        while (moisturePercentage <= 50 && millis() - startTime <= 100000) {  // 100 seconds
            delay(1000);
            unsigned long currentTime = millis();
            if (currentTime - startTime > 100000) {  // 100 seconds
                Serial.println("Safety Timer Exceeded");
                break; // Exit the loop if time exceeds 100 seconds
            }
            Serial.print("Watering count: ");
            Serial.println(currentTime - startTime);
            moisturePercentage = updateAndSendMoisture(channel, "on", false); // Recalculate moisture
        }
    }
    digitalWrite(pinNum, HIGH); // Disengage relay
    unsigned long endTime = millis();  // Capture the end time right after the loop
    safetyCheck(channel, startTime, endTime);
}


void safetyCheck(int channel, unsigned long startTime, unsigned long endTime) {
    unsigned long wateringDuration = endTime - startTime;
    if (wateringDuration >= 100000) {  // 100 seconds
        sensors[channel].safetyFlag = true; // Set flag true here
        updateAndSendMoisture(channel, "off", sensors[channel].safetyFlag);
        sendData(channel);
    }
}

int updateAndSendMoisture(int channel, const char* state, bool safetyFlag) {
    Serial.print("Channel: ");
    Serial.println(channel);
    yield();  // Yield to maintain system health
    int16_t adcValue = ads.readADC_SingleEnded(channel);  // Get new reading from pin
    yield();  // Yield to maintain system health
    int moisturePercentage = map(adcValue, 448, 938, 100, 0);  // Convert reading to percentage
    Serial.print("Moisture: ");
    Serial.print(moisturePercentage);
    Serial.println("%");
    updateSensor(channel, moisturePercentage, state, safetyFlag);
    Serial.println("Sensor Updated");
    yield();  // Yield before potentially blocking operations
    sendData(channel);
    saveSensorData();
    return moisturePercentage;
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

    sensors[channel].safetyFlag = safetyFlag;
}


void sendData(int channel) {
    Serial.println("Send Data Function");
  DynamicJsonDocument doc(1024);
  JsonObject data = doc.createNestedObject("data");
  data["sensorNumber"] = sensors[channel].sensorNumber;
  data["moisture"] = sensors[channel].moisture;
  data["date"] = sensors[channel].date;
  data["state"] = sensors[channel].state;
  data["safetyFlag"] = sensors[channel].safetyFlag;

  sendJsonData(doc);  // Assuming this function sends the JSON to your server
}


void getTime(char* buffer, size_t bufferSize) {
    time_t now = time(nullptr);  // Get current time
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, bufferSize, "%d-%m-%Y %H:%M:%S", timeinfo);
}