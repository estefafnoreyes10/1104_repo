// void setup() {
//   Serial.begin(9600); // open serial port, set the baud rate as 9600 bps
// }
// void loop() {
//   int val;
//   val = analogRead(0); //connect sensor to Analog 0
//   Serial.println(val); //print the value to serial port
//   delay(100);
// }


// Define sensor reading thresholds
const int AirValue = 3500;   // you need to replace this value with Value_1
const int WaterValue = 1500;  // you need to replace this value with Value_2
int intervals = (AirValue - WaterValue) / 3;  // Calculate intervals
int soilMoistureValue = 0;  // Variable to store the moisture value

void setup() {
  Serial.begin(9600); // Open serial port, set the baud rate to 9600 bps
}
void loop() {
  soilMoistureValue = analogRead(A0);  // Read the sensor value from Analog pin 0
  
  Serial.print("Raw Value: ");
  Serial.println(soilMoistureValue);  // Always show the raw value

  // Check moisture levels
  if (soilMoistureValue <= WaterValue) {
    Serial.println("Water");
  } else if (soilMoistureValue <= WaterValue + intervals) {
    Serial.println("Very Wet");
  } else if (soilMoistureValue <= AirValue - intervals) {
    Serial.println("Wet");
  } else if (soilMoistureValue < AirValue) {
    Serial.println("Dry");
  } else {
    Serial.println("Air");
  }

  delay(100);  // Short delay to prevent serial output flooding
}
