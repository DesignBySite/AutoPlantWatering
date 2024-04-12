# 1 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
# 2 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 3 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 4 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 5 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 6 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 7 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2

const char* ssid = "Kevin Home 2.4";
const char* password = "nrpxKReM84!!";
const char* serverUrl = "http://10.0.0.252:3050/data";

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
  Serial.println("Kevin Home 2.4");

  ads.begin(0x48);
  pinMode(D3, 0x01); // Set the D3 pin as an output
  pinMode(D4, 0x01); // Set the D4 pin as an output
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
      digitalWrite(pinNum, 0x0); // engage relay

      while (moisturePercentage <= 60) {
        delay(1000);
        adcValue = ads.readADC_SingleEnded(channel); // get new reading from pin
        moisturePercentage = map(adcValue, wettest, dryest, 100, 0); //recalculate moisture
        sendData(channel, moisturePercentage);
      };
  }
  digitalWrite(pinNum, 0x1); // disengage relay
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
