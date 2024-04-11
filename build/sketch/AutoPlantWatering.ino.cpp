#include <Arduino.h>
#line 1 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1015 ads;

#define D3_PIN D3 // Define the pin connected to the relay

#line 7 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void setup();
#line 13 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void loop();
#line 37 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void engageWatering(int channel, int pinNum);
#line 60 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void moisturePrint(int channel, int moisture);
#line 7 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void setup() {
  Serial.begin(9600); // Initialize serial communication
  ads.begin(0x48);
  pinMode(D3_PIN, OUTPUT); // Set the D3 pin as an outputP
}

void loop() {
  // Call the engageWatering function only when a certain condition is met.
  // For example, you could use a button press, a sensor threshold, or just a simple delay as shown here.


  // printMoistureReading(0);
  engageWatering(0, D3_PIN); // Call the watering function
  engageWatering(1, D4);

  delay(3000); // Wait for 30 seconds before calling the function again. Adjust as per your requirement.
}

// void printMoistureReading(int channel) {
//   int16_t adc1;
//   adc1 = ads.readADC_SingleEnded(channel);



//   Serial.print("Moisture Level ADC Value pin 1: ");
//   Serial.println(adc1);
// }
/************************************/
/** Make this reusable for each pin */
/************************************/
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
  digitalWrite(pinNum, HIGH);
}

void moisturePrint(int channel, int moisture) {
  Serial.print("Moisture Level ADC Value pin ");
  Serial.print(channel);
  Serial.print(": ");
  Serial.print(moisture);
  Serial.println("%");
}

