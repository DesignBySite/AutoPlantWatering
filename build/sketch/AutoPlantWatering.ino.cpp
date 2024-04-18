#include <Arduino.h>
#line 1 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <time.h>
#include <string>
#include "./secrets.h"

using namespace std;

const char* ssid = SECRET_WIFI;
const char* password = SECRET_PASS;
const char* serverUrl = SECRET_SERVER;

Adafruit_ADS1015 ads;
WiFiClient wifiClient;
HTTPClient http;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
struct SensorData {
    int sensorNumber;
    int moisture;
    string date;  
    string state; 
    bool safetyFlag;
};

SensorData sensors[4];

#line 34 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void saveSensorData();
#line 45 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void loadSensorData();
#line 55 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void setup();
#line 81 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void loop();
#line 91 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
int updateAndSendMoisture(int channel, string state, bool safetyFlag);
#line 100 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void engageWatering(int channel, int pinNum);
#line 134 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void sendJsonData(DynamicJsonDocument& doc);
#line 146 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void updateSensor(int channel, int moisture, string state, bool safetyFlag);
#line 155 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void sendData(int channel);
#line 168 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
string getTime();
#line 34 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void saveSensorData() {
    int startAddress = 0;
    for (int i = 0; i < 4; i++) {
        EEPROM.put(startAddress, sensors[i].sensorNumber);
        startAddress += sizeof(sensors[i].sensorNumber);
        EEPROM.put(startAddress, sensors[i].safetyFlag);
        startAddress += sizeof(sensors[i].safetyFlag);
    }
    EEPROM.commit();
}

void loadSensorData() {
    int startAddress = 0;
    for (int i = 0; i < 4; i++) {
        EEPROM.get(startAddress, sensors[i].sensorNumber);
        startAddress += sizeof(sensors[i].sensorNumber);
        EEPROM.get(startAddress, sensors[i].safetyFlag);
        startAddress += sizeof(sensors[i].safetyFlag);
    }
}

void setup() {
  Serial.begin(9600); // Initialize serial communication
  WiFi.begin(ssid, password);
  timeClient.begin();
  timeClient.update();

  configTime(0, 0, "pool.ntp.org"); // Initialize the time library to get the time


  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  };
  
  delay(1000);
  Serial.print("Connected Successfully to ");
  Serial.println(SECRET_WIFI);

  EEPROM.begin(512);  // Ensure enough space is allocated
  loadSensorData();  // Load sensor data from EEPROM

  ads.begin(0x48);
  pinMode(D3, OUTPUT); // Set the D3 pin as an output
  pinMode(D4, OUTPUT); // Set the D4 pin as an output
}

void loop() {
  loadSensorData();
  delay(1000);
  engageWatering(0, D3); // Call the watering function
  delay(1000);
  engageWatering(1, D4);

  delay(30000); // Wait for 30 seconds before calling the function again. Adjust as per your requirement.
}

int updateAndSendMoisture(int channel, string state, bool safetyFlag) {
  int16_t adcValue = ads.readADC_SingleEnded(channel);// get new reading from pin
  int moisturePercentage = map(adcValue, 448, 938, 100, 0);  // Assuming these are constants calculate moisture into a decimal
  updateSensor(channel, moisturePercentage, state, safetyFlag);
  sendData(channel);
  saveSensorData();
  return moisturePercentage;
}

void engageWatering(int channel, int pinNum) {
  if (sensors[channel].safetyFlag) {
    digitalWrite(pinNum, HIGH); // Disengage relay
    Serial.print("Warning, check status of sensor: ");
    Serial.println(channel);
    return;
  }

  Serial.print("Engaging watering for channel: ");
  Serial.println(channel);
  
  int moisturePercentage = updateAndSendMoisture(channel, "off", false);
  Serial.print("Moisture: ");
  Serial.println(moisturePercentage);
  int safetyTimer = 0;

  if (moisturePercentage <= 10 && !sensors[channel].safetyFlag) {
      digitalWrite(pinNum, LOW); // Engage relay
      while (moisturePercentage <= 60 && safetyTimer <= 10) {
        delay(1000);
        safetyTimer++;
        moisturePercentage = updateAndSendMoisture(channel, "on", false); //recalculate moisture
      }
  }

  digitalWrite(pinNum, HIGH); // Disengage relay

  if (safetyTimer >= 10) {
    sensors[channel].safetyFlag = true; // Set flag true here
    updateAndSendMoisture(channel, "off", sensors[channel].safetyFlag);
    sendData(channel);
  }
}

void sendJsonData(DynamicJsonDocument& doc) {
  String jsonObject;
  serializeJson(doc, jsonObject);

  http.begin(wifiClient, serverUrl);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonObject);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();
}
// Example of updating sensor data
void updateSensor(int channel, int moisture, string state, bool safetyFlag) {
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


string getTime() {
  time_t now = time(nullptr);  // Get current time
  struct tm* timeinfo = localtime(&now);

  char buffer[80];
  strftime(buffer, sizeof(buffer), "%m-%d-%Y %H:%M:%S", timeinfo); // Format date and time
  return std::string(buffer);
}
