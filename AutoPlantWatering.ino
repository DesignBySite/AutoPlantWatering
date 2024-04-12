#include <Adafruit_ADS1X15.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "./secrets.h"

const char* ssid = SECRET_WIFI;
const char* password = SECRET_PASS;
const char* serverUrl = SECRET_SERVER;

Adafruit_ADS1015 ads;
WiFiClient wifiClient;
HTTPClient http;

void setup() {
  Serial.begin(9600); // Initialize serial communication
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  };
  
  delay(1000);
  Serial.print("Connected Successfully to ");
  Serial.println(SECRET_WIFI);
  
  ads.begin(0x48);
  pinMode(D3, OUTPUT); // Set the D3 pin as an output
  pinMode(D4, OUTPUT); // Set the D4 pin as an output
}

void loop() {
  engageWatering(0, D3); // Call the watering function
  engageWatering(1, D4);

  delay(30000); // Wait for 30 seconds before calling the function again. Adjust as per your requirement.
}

int updateAndSendMoisture(int channel) {
  int16_t adcValue = ads.readADC_SingleEnded(channel);// get new reading from pin
  int moisturePercentage = map(adcValue, 448, 938, 100, 0);  // Assuming these are constants calculate moisture into a decimal
  sendData(channel, moisturePercentage);
}

void engageWatering(int channel, int pinNum) {
  int moisturePercentage = updateAndSendMoisture(channel);

  if (moisturePercentage <= 15) {
      digitalWrite(pinNum, LOW); // Engage relay
      while (moisturePercentage <= 60) {
        delay(1000);
        moisturePercentage = updateAndSendMoisture(channel); //recalculate moisture
      }
      digitalWrite(pinNum, HIGH); // Disengage relay
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

void sendData(int channel, int moisture) {
  DynamicJsonDocument doc(1024);
  JsonObject sensor = doc.createNestedObject("sensor");
  sensor["number"] = channel;
  sensor["value"] = moisture;

  sendJsonData(doc);
}
