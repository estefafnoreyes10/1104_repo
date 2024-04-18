// Constants for soil moisture sensor thresholds
const int AirValue = 3500;   // Replace with the maximum dry value (air)
const int WaterValue = 1500; // Replace with the minimum wet value (water)
int intervals = (AirValue - WaterValue) / 3;  // Calculate intervals to classify moisture level
int soilMoistureValue = 0;  // Variable to store the moisture value

// Include the library for the DHT22 sensor
#include <DHT22.h>
#define pinDATA 4 // Define the pin where the data line of the DHT22 sensor is connected

DHT22 dht22(pinDATA); // Initialize the DHT22 sensor

void setup() {
  Serial.begin(115200); // Open serial port, set the baud rate to 115200 bps
  Serial.println("Starting sensors test...");
}

void loop() {
  // Read and process soil moisture data
  soilMoistureValue = analogRead(A0);  // Read the value from Analog pin 0
  Serial.print("Soil Moisture Raw Value: ");
  Serial.println(soilMoistureValue);  // Display the raw value of soil moisture
  
  // Classify and display the moisture level
  if (soilMoistureValue <= WaterValue) {
    Serial.println("Status: Water");
  } else if (soilMoistureValue <= WaterValue + intervals) {
    Serial.println("Status: Very Wet");
  } else if (soilMoistureValue <= AirValue - intervals) {
    Serial.println("Status: Wet");
  } else if (soilMoistureValue < AirValue) {
    Serial.println("Status: Dry");
  } else {
    Serial.println("Status: Air");
  }

  // Read and process temperature and humidity data
  Serial.println(dht22.debug()); // Optional: Output debug information from the DHT22

  float temperature = dht22.getTemperature();
  float humidity = dht22.getHumidity();

  // Handle potential errors from DHT22 sensor
  if (dht22.getLastError() != dht22.OK) {
    Serial.print("Last error: ");
    Serial.println(dht22.getLastError());
  }

  // Display temperature and humidity
  Serial.print("Humidity: ");
  Serial.print(humidity, 1);
  Serial.print("%\tTemperature: ");
  Serial.print(temperature, 1);
  Serial.println("°C");

  delay(2000);  // Delay to prevent flooding the serial output
}
