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


// const char* ssid = SECRET_WIFI;
// const char* password = SECRET_PASS;
// const char* serverUrl = SECRET_SERVER;
// int safetyTimer = 0;


// NTPClient timeClient(ntpUDP);


// SensorData sensors[4];
// ESP8266WebServer server(80);

// /***        NOTES       **/
// // Active is triggered by LOW
// // D3 is GPIO 0


// void handlePost() {
//   if (server.hasArg("plain")) {
//     String body = server.arg("plain");
//     DynamicJsonDocument doc(1024);
//     deserializeJson(doc, body);
//     int sensorNumber = doc["sensorNumber"];
//     bool safetyFlag = doc["safetyFlag"];

//     sensors[sensorNumber].safetyFlag = safetyFlag;
//     saveSensorData();
//     server.send(200, "application/json", "{\"status\":\"received\"}");
//   } else {
//     server.send(500, "text/plain", "Server Error: Missing Data");
//   }
// }

// void saveSensorData() {
//     int startAddress = 0;
//     for (int i = 0; i < 4; i++) {
//         EEPROM.put(startAddress, sensors[i].sensorNumber);
//         startAddress += sizeof(sensors[i].sensorNumber);
//         EEPROM.put(startAddress, sensors[i].safetyFlag);
//         startAddress += sizeof(sensors[i].safetyFlag);
//     }
//     EEPROM.commit();
// }

// void loadSensorData() {
//     int startAddress = 0;
//     for (int i = 0; i < 4; i++) {
//         EEPROM.get(startAddress, sensors[i].sensorNumber);
//         startAddress += sizeof(sensors[i].sensorNumber);
//         EEPROM.get(startAddress, sensors[i].safetyFlag);
//         startAddress += sizeof(sensors[i].safetyFlag);
//     }
// }

// void setup() {
//   Serial.begin(9600); // Initialize serial communication
//   WiFi.begin(ssid, password);
//   timeClient.begin();
//   timeClient.update();

//   configTime(0, 0, "pool.ntp.org"); // Initialize the time library to get the time


//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.println("Connecting to WiFi...");
//   };
  
//   delay(1000);



//   delay(1000);

//   EEPROM.begin(512);  // Ensure enough space is allocated

//   server.on("/updateFlag", HTTP_POST, handlePost);  // Setup the path and handler
//   server.begin(); // Start the server
//   Serial.print("HTTP server started");
//   Serial.println(WiFi.localIP());

//   ads.begin(0x48);
//   pinMode(D3, OUTPUT); // Set the D3 pin as an output
//   pinMode(D4, OUTPUT); // Set the D4 pin as an output
//   pinMode(D5, OUTPUT); // Set the D4 pin as an output
//   pinMode(D6, OUTPUT); // Set the D4 pin as an output
// }

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

struct SensorData {
    int sensorNumber;
    int moisture;
    String date;
    String state;
    bool safetyFlag;
};

SensorData sensors[4];

void handlePost() {
    if (!server.hasArg("plain")) {
        server.send(500, "text/plain", "Server Error: Missing Data");
        return;
    }
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, body);
    int sensorNumber = doc["sensorNumber"];
    sensors[sensorNumber].safetyFlag = doc["safetyFlag"];
    saveSensorData();
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
  }
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
    adsBegin();
    delay(2000);
    wifiConnect();

    Serial.println("Initializing further components...");
    EEPROM.begin(512);

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
    if (millis() - lastWateringTime >= 100) {
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
  safetyCheck(channel);
 
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
            Serial.print("Waterting count: ");
            Serial.println(millis() - startTime);
            moisturePercentage = updateAndSendMoisture(channel, "on", false); // Recalculate moisture
        }
    }
    digitalWrite(pinNum, HIGH); // Disengage relay
}


void safetyCheck(int channel) {
    if (millis() - startTime >= 100000) {  // 100 seconds
        sensors[channel].safetyFlag = true; // Set flag true here
        updateAndSendMoisture(channel, "off", sensors[channel].safetyFlag);
        sendData(channel);
    }
}

int updateAndSendMoisture(int channel, String state, bool safetyFlag) {
    Serial.print("Channel: ");
    Serial.println(channel);
    yield();  // Yield to maintain system health
    int16_t adcValue = ads.readADC_SingleEnded(channel);  // Get new reading from pin
    Serial.print("ADC Value: ");
    Serial.println(adcValue);
    yield();  // Yield to maintain system health
    int moisturePercentage = map(adcValue, 448, 938, 100, 0);  // Convert reading to percentage
    updateSensor(channel, moisturePercentage, state, safetyFlag);
    yield();  // Yield before potentially blocking operations
    sendData(channel);
    saveSensorData();
    return moisturePercentage;
}


void sendJsonData(DynamicJsonDocument& doc) {
    String jsonObject;
    serializeJson(doc, jsonObject);
    http.begin(wifiClient, serverUrl);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonObject);
    if (httpResponseCode != 200) {
        Serial.print("HTTP Request failed: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}


void updateSensor(int channel, int moisture, String state, bool safetyFlag) {
  sensors[channel].sensorNumber = channel;
  sensors[channel].moisture = moisture;
  sensors[channel].date = getTime(); // Assuming getTime() returns a std::string
  sensors[channel].state = state;
  sensors[channel].safetyFlag = safetyFlag;
}


void sendData(int channel) {
  DynamicJsonDocument doc(1024);
  JsonObject data = doc.createNestedObject("data");
  data["sensorNumber"] = sensors[channel].sensorNumber;
  data["moisture"] = sensors[channel].moisture;
  data["date"] = sensors[channel].date;
  data["state"] = sensors[channel].state;
  data["safetyFlag"] = sensors[channel].safetyFlag;

  sendJsonData(doc);  // Assuming this function sends the JSON to your server
}


String getTime() {
    time_t now = time(nullptr); // Get current time
    struct tm* timeinfo = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%m-%d-%Y %H:%M:%S", timeinfo);
    return String(buffer);
}