#include <Arduino.h>
#line 1 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
#define D3_PIN D3 // Define the pin connected to the relay

#line 3 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void setup();
#line 8 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void loop();
#line 17 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void engageWatering();
#line 3 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"
void setup() {
  Serial.begin(9600); // Initialize serial communication
  pinMode(D3_PIN, OUTPUT); // Set the D3 pin as an output
}

void loop() {
  // Call the engageWatering function only when a certain condition is met.
  // For example, you could use a button press, a sensor threshold, or just a simple delay as shown here.
  
  engageWatering(); // Call the watering function
  
  delay(30000); // Wait for 30 seconds before calling the function again. Adjust as per your requirement.
}

void engageWatering() {
  int num = 10;
  while(num > 0) {
    Serial.print("Count ");
    Serial.println(num);
    num--;
    delay(1000); // Use delay() for a 1-second pause
  }
  digitalWrite(D3_PIN, LOW); // Activate the relay after the countdown
  delay(5000); // Keep the relay activated for 5 seconds. Adjust the duration as needed.
  digitalWrite(D3_PIN, HIGH); // Deactivate the relay
}

