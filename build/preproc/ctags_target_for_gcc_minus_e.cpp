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


  // printMoistureReading(0);
  engageWatering(0, D3 /* Define the pin connected to the relay*/); // Call the watering function
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
      digitalWrite(pinNum, 0x0); // engage relay

      while (moisturePercentage <= 60) {
        delay(1000);
        adcValue = ads.readADC_SingleEnded(channel); // get new reading from pin
        moisturePercentage = map(adcValue, wettest, dryest, 100, 0); //recalculate moisture
        moisturePrint(channel, moisturePercentage);
      };
  }
  digitalWrite(pinNum, 0x1);
}

void moisturePrint(int channel, int moisture) {
  Serial.print("Moisture Level ADC Value pin ");
  Serial.print(channel);
  Serial.print(": ");
  Serial.print(moisture);
  Serial.println("%");
}
