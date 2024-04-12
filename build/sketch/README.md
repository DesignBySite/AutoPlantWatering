#line 1 "/Users/kevinhome/AutoPlantWatering/README.md"
# Indoor Garden Watering System

This project automates watering for indoor gardens using the ESP8266 and Adafruit ADS1015 sensor, monitoring soil moisture and watering plants as needed.

## Hardware Required

- ESP8266 Microcontroller
- Adafruit ADS1015 Analog-to-Digital Converter
- Soil Moisture Sensors
- Relay Modules
- Wires and Connectors

## Features

- WiFi connectivity to send data to a server.
- Real-time soil moisture monitoring.
- Automatic watering based on moisture thresholds.

## Setup

1. **WiFi Configuration**: Define your WiFi credentials in `secrets.h`.
2. **Server Configuration**: Set your server's URL in the code.
3. **Sensor and Relay Setup**: Connect your moisture sensors and relay modules to the ESP8266.

## Usage

The system reads soil moisture levels using the ADS1015 sensor. It starts watering if the moisture level drops below 15% and stops once it reaches 60%.

### Function Descriptions

- `engageWatering(int channel, int pinNum)`: Manages watering based on the sensor input from a specified channel.
- `moisturePrint(int channel, int moisture)`: Outputs the moisture level to the serial monitor.

## Serial Output

Moisture levels are displayed as "Moisture Level ADC Value pin X: Y%".

Refer to the system documentation and Adafruit's ADS1015 datasheet for detailed setup and deployment instructions.
