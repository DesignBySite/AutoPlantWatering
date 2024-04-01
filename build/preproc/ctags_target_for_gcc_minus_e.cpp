# 1 "/Users/kevinhome/AutoPlantWatering/AutoPlantWatering.ino"


void setup() {
  Serial.begin(9600); // Initialize serial communication
  pinMode(D3 /* Define the pin connected to the relay*/, 0x01); // Set the D3 pin as an output
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
  digitalWrite(D3 /* Define the pin connected to the relay*/, 0x0); // Activate the relay after the countdown
  delay(5000); // Keep the relay activated for 5 seconds. Adjust the duration as needed.
  digitalWrite(D3 /* Define the pin connected to the relay*/, 0x1); // Deactivate the relay
}
