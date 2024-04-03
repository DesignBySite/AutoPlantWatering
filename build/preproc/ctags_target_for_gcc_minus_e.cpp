# 1 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
# 2 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino" 2

Adafruit_ADS1015 ads;



void setup() {
  Serial.begin(9600); // Initialize serial communication
  ads.begin(0x48);
  pinMode(D3 /* Define the pin connected to the relay*/, 0x01); // Set the D3 pin as an outputP
}

void loop() {
  // Call the engageWatering function only when a certain condition is met.
  // For example, you could use a button press, a sensor threshold, or just a simple delay as shown here.


  printMoistureReading();
  engageWatering(); // Call the watering function

  delay(3000); // Wait for 30 seconds before calling the function again. Adjust as per your requirement.
}

void printMoistureReading() {
  int16_t adc0, adc1;
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);

  Serial.print("Moisture Level ADC Value pin 0: ");
  Serial.println(adc0);

  Serial.print("Moisture Level ADC Value pin 1: ");
  Serial.println(adc1);
}

void engageWatering() {
  int dryest = 938;
  int wettest = 448;
   // your sensor reading;
  int16_t adc0;
  adc0 = ads.readADC_SingleEnded(0);
  // Map the reading to a percentage
  int moisturePercentage = map(adc0, wettest, dryest, 100, 0);

  if (moisturePercentage <= 10) {
      // Start something
      digitalWrite(D3 /* Define the pin connected to the relay*/, 0x0);
      do {
        delay(1000);
        adc0 = ads.readADC_SingleEnded(0);

        Serial.print("ADC0 read: ");
        Serial.println(adc0);

        moisturePercentage = map(adc0, wettest, dryest, 100, 0);

        Serial.print("Watering at ");
        Serial.println(moisturePercentage);
        Serial.println("%");

      } while (moisturePercentage <= 60);
      digitalWrite(D3 /* Define the pin connected to the relay*/, 0x1);
  }
}
