#include <Arduino.h>
#line 1 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
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

#line 16 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void setup();
#line 34 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void loop();
#line 45 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void engageWatering(int channel, int pinNum);
#line 66 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void sendData(int channel, int moisture);
#line 16 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
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

/*****************/
/* Uncomment once done with Wifi */
/*****************/
  engageWatering(0, D3); // Call the watering function
  engageWatering(1, D4);

  delay(3000); // Wait for 3 seconds before calling the function again. Adjust as per your requirement.
}

void engageWatering(int channel, int pinNum) {
  int dryest = 938;
  int wettest = 448;
  int16_t adcValue = ads.readADC_SingleEnded(channel);
  int moisturePercentage = map(adcValue, wettest, dryest, 100, 0); //calculate moisture into a decimal

  sendData(channel, moisturePercentage);

  if (moisturePercentage <= 15) {
      digitalWrite(pinNum, LOW); // engage relay

      while (moisturePercentage <= 60) {
        delay(1000);
        adcValue = ads.readADC_SingleEnded(channel); // get new reading from pin
        moisturePercentage = map(adcValue, wettest, dryest, 100, 0); //recalculate moisture
        sendData(channel, moisturePercentage);
      };
  }
  digitalWrite(pinNum, HIGH); // disengage relay
}

void sendData(int channel, int moisture) {

  DynamicJsonDocument doc(1024);
  JsonObject sensor = doc.createNestedObject("sensor");
  String jsonObject;

  sensor["number"] = channel;
  sensor["value"] = moisture;

  serializeJson(doc, jsonObject);

  http.begin(wifiClient, serverUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonObject); // Replace with your actual data
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  http.end();
}

