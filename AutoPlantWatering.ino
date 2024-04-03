#include <Adafruit_ADS1X15.h>

Adafruit_ADS1015 ads;

#define D3_PIN D3 // Define the pin connected to the relay

void setup() {
  Serial.begin(9600); // Initialize serial communication
  ads.begin(0x48);
  pinMode(D3_PIN, OUTPUT); // Set the D3 pin as an outputP
}

void loop() {
  // Call the engageWatering function only when a certain condition is met.
  // For example, you could use a button press, a sensor threshold, or just a simple delay as shown here.


  printMoistureReading();
  engageWatering(); // Call the watering function
  
  delay(3000); // Wait for 30 seconds before calling the function again. Adjust as per your requirement.
}

void printMoistureReading() {
  int16_t adc1;
  adc1 = ads.readADC_SingleEnded(1);

  

  Serial.print("Moisture Level ADC Value pin 1: ");
  Serial.println(adc1);
}

void engageWatering() {
  int dryest = 938;
  int wettest = 448;

  int16_t adc0 = ads.readADC_SingleEnded(0);

  int moisturePercentage = map(adc0, wettest, dryest, 100, 0); //calculate moisture into a decimal
  Serial.print("Moisture Level ADC Value pin 0: ");
  Serial.println(moisturePercentage);
  Serial.println("%");

  if (moisturePercentage <= 10) {
      digitalWrite(D3_PIN, LOW); // engage relay
      do {
        delay(1000);
        adc0 = ads.readADC_SingleEnded(0); // get new reading from pin

        moisturePercentage = map(adc0, wettest, dryest, 100, 0); //recalculate moisture

        Serial.print("Watering at ");
        Serial.println(moisturePercentage);
        Serial.println("%");

      } while (moisturePercentage <= 60);
      digitalWrite(D3_PIN, HIGH);
  }
}
