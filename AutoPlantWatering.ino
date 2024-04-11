#include <Adafruit_ADS1X15.h>

Adafruit_ADS1015 ads;

void setup() {
  Serial.begin(9600); // Initialize serial communication
  ads.begin(0x48);
  pinMode(D3, OUTPUT); // Set the D3 pin as an output
  pinMode(D4, OUTPUT); // Set the D4 pin as an output
}

void loop() {
  engageWatering(0, D3); // Call the watering function
  engageWatering(1, D4);

  delay(3000); // Wait for 3 seconds before calling the function again. Adjust as per your requirement.
}

void engageWatering(int channel, int pinNum) {
  int dryest = 938;
  int wettest = 448;
  int16_t adcValue = ads.readADC_SingleEnded(channel);
  int moisturePercentage = map(adcValue, wettest, dryest, 100, 0); //calculate moisture into a decimal

  moisturePrint(channel, moisturePercentage);

  if (moisturePercentage <= 15) {
      digitalWrite(pinNum, LOW); // engage relay

      while (moisturePercentage <= 60) {
        delay(1000);
        adcValue = ads.readADC_SingleEnded(channel); // get new reading from pin
        moisturePercentage = map(adcValue, wettest, dryest, 100, 0); //recalculate moisture
        moisturePrint(channel, moisturePercentage);
      };
  }
  digitalWrite(pinNum, HIGH); // disengage relay
}

void moisturePrint(int channel, int moisture) {
  Serial.print("Moisture Level ADC Value pin ");
  Serial.print(channel);
  Serial.print(": ");
  Serial.print(moisture);
  Serial.println("%");
}