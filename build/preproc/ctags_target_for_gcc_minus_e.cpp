# 1 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
# 2 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 3 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 4 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 5 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 6 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 7 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 8 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 9 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 10 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 11 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 12 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2
# 13 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2


# 14 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
using namespace std;

const char* ssid = "Kevin Home 2.4";
const char* password = "nrpxKReM84!!";
const char* serverUrl = "http://10.0.0.252:3050/data";

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
ESP8266WebServer server(80);

void handlePost() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, body);
    int sensorNumber = doc["sensorNumber"];
    bool safetyFlag = doc["safetyFlag"];

    sensors[sensorNumber].safetyFlag = safetyFlag;
    saveSensorData();
    server.send(200, "application/json", "{\"status\":\"received\"}");
  } else {
    server.send(500, "text/plain", "Server Error: Missing Data");
  }
}

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
  Serial.println("Kevin Home 2.4");
  delay(1000);
  EEPROM.begin(512); // Ensure enough space is allocated

  server.on("/updateFlag", HTTP_POST, handlePost); // Setup the path and handler
  server.begin(); // Start the server
  Serial.print("HTTP server started");
  Serial.println(WiFi.localIP());

  ads.begin(0x48);
  pinMode(D3, 0x01); // Set the D3 pin as an output
  pinMode(D4, 0x01); // Set the D4 pin as an output
}

void loop() {
  server.handleClient();
  Serial.println("1");
  loadSensorData();
  Serial.println("2");
  delay(1000);
  //engageWatering(0, D3); // Call the watering function
  delay(1000);
  digitalWrite(D3, 0x0);
  digitalWrite(D4, 0x1);
  Serial.println("written");
  //engageWatering(1, D4);

  delay(30000); // Wait for 30 seconds before calling the function again. Adjust as per your requirement.
}

int updateAndSendMoisture(int channel, string state, bool safetyFlag) {
  Serial.println("3");
  int16_t adcValue = ads.readADC_SingleEnded(channel);// get new reading from pin
  Serial.print("Sensor Reading: ");
  Serial.println(adcValue);
  int moisturePercentage = map(adcValue, 448, 938, 100, 0); // Assuming these are constants calculate moisture into a decimal
  Serial.print("Moisture Reading: ");
  Serial.println(moisturePercentage);
  updateSensor(channel, moisturePercentage, state, safetyFlag);
  sendData(channel);
  saveSensorData();
  return moisturePercentage;
}

void engageWatering(int channel, int pinNum) {
  if (sensors[channel].safetyFlag) {
    Serial.print("Channel off: ");
    Serial.println(channel);
    digitalWrite(pinNum, 0x1); // Disengage relay
    return;
  }
  Serial.print("Channel: ");
  Serial.println(channel);
  int moisturePercentage = updateAndSendMoisture(channel, "off", false);
  int safetyTimer = 0;

  if (moisturePercentage <= 10 && !sensors[channel].safetyFlag) {
      digitalWrite(pinNum, 0x0); // Engage relay
      while (moisturePercentage <= 50 && safetyTimer <= 5) {
        delay(1000);
        safetyTimer++;
        moisturePercentage = updateAndSendMoisture(channel, "on", false); //recalculate moisture
      }
  }

  digitalWrite(pinNum, 0x1); // Disengage relay

  if (safetyTimer >= 5) {
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

    sendJsonData(doc); // Assuming this function sends the JSON to your server
}


string getTime() {
  time_t now = time(nullptr); // Get current time
  struct tm* timeinfo = localtime(&now);

  char buffer[80];
  strftime(buffer, sizeof(buffer), "%m-%d-%Y %H:%M:%S", timeinfo); // Format date and time
  return std::string(buffer);
}