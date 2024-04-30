//#include <DHT22.h>

//one param of how dry 

// Constants for soil moisture sensor thresholds
const int AirValue = 3207;   // maximum dry value
const int WaterValue = 2200; // minimum wet value
int intervals = (AirValue - WaterValue) / 3;
int soilMoistureValue = 0;

//#define pinDATA 4
//DHT22 dht22(pinDATA);

const int relayControlPin = 25; // Relay control pin for the pump

void setup() {
  pinMode(relayControlPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("Starting sensors test...");
}

void loop() {
  // Read and process soil moisture data
  soilMoistureValue = analogRead(A0);
  Serial.print("Soil Moisture Raw Value: ");
  Serial.println(soilMoistureValue);

  // Display the moisture level and control the pump
  if (soilMoistureValue <= WaterValue) {
    Serial.println("Status: Water");
    digitalWrite(relayControlPin, LOW); // Soil is very wet, turn off the pump
  } else if (soilMoistureValue <= WaterValue + intervals) {
    Serial.println("Status: Very Wet");
    Serial.println("Pump: OFF VERY WET");
    digitalWrite(relayControlPin, LOW); // Still wet enough, keep pump off
  } else if (soilMoistureValue <= AirValue - intervals) {
    Serial.println("Status: Wet");
    Serial.println("Pump: OFF WET");
    digitalWrite(relayControlPin, LOW); // Moist enough, no watering needed
  } else if (soilMoistureValue < AirValue) {
    Serial.println("Status: Dry");
    digitalWrite(relayControlPin, HIGH); // Dry, turn on the pump
    Serial.println("Pump: ON DRY");
  } else {
    Serial.println("Status: Air");
    digitalWrite(relayControlPin, HIGH); // Very dry, definitely turn on the pump
    Serial.println("Pump: ON VERY DRY");
  }

  // // Read temperature and humidity
  // float temperature = dht22.getTemperature();
  // float humidity = dht22.getHumidity();
  // Serial.print("Humidity: ");
  // Serial.print(humidity, 1);
  // Serial.print("%\tTemperature: ");
  // Serial.print(temperature, 1);
  // Serial.println("°C");

  delay(10000); // Check every 10 seconds
}