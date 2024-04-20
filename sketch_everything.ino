// Constants for soil moisture sensor thresholds
const int AirValue = 3500;   // Replace with the maximum dry value (air)
const int WaterValue = 1500; // Replace with the minimum wet value (water)
int intervals = (AirValue - WaterValue) / 3;  // Calculate intervals to classify moisture level
int soilMoistureValue1  = 0;  // Variable to store the moisture value
int soilMoistureValue2 = 0;
int soilMoistureValue3 = 0;
int soilMoistureValue4 = 0;
int soilMoistureValue5 = 0;


// Include the library for the DHT22 sensor
#include <DHT22.h>
#define pinDATA 4 // Define the pin where the data line of the DHT22 sensor is connected



DHT22 dht22(4); // Initialize the DHT22 sensor

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include "SparkFun_SCD4x_Arduino_Library.h"
SCD4x mySensor;
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  Serial.println("");

  //co2
  Wire.begin();


  delay(500);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */  
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("13 ms");
  Serial.println("------------------------------------");
}




void setup() {
  Serial.begin(115200); // Open serial port, set the baud rate to 115200 bps
  Serial.println("Starting sensors test...");

    /* Display some basic information on this sensor */
  displaySensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  /* We're ready to go! */
  Serial.println("");
}

void loop() {
  // Read and process soil moisture data
  soilMoistureValue1 = analogRead(A0);  // Read the value from Analog pin 0
  soilMoistureValue2 = analogRead(A3);
  soilMoistureValue3 = analogRead(A6);
  soilMoistureValue4 = analogRead(A7);
  soilMoistureValue5 = analogRead(A4);


  Serial.print("Soil Moisture Raw Value for sensor 1: ");
  Serial.println(soilMoistureValue1);  // Display the raw value of soil moisture
  Serial.println("################");
  Serial.print("Soil Moisture Raw Value for sensor 2: ");
  Serial.println(soilMoistureValue2);  // Display the raw value of soil moisture
  Serial.println("################");
  Serial.print("Soil Moisture Raw Value for sensor 3: ");
  Serial.println(soilMoistureValue3);  // Display the raw value of soil moisture
  Serial.println("################");
  Serial.print("Soil Moisture Raw Value for sensor 4: ");
  Serial.println(soilMoistureValue4);  // Display the raw value of soil moisture
  Serial.println("################");
  Serial.print("Soil Moisture Raw Value for sensor 5: ");
  Serial.println(soilMoistureValue5);  // Display the raw value of soil moisture

  
  // Classify and display the moisture level
  if (soilMoistureValue1 <= WaterValue) {
    Serial.println("Status: Water");
  } else if (soilMoistureValue1 <= WaterValue + intervals) {
    Serial.println("Status: Very Wet");
  } else if (soilMoistureValue1 <= AirValue - intervals) {
    Serial.println("Status: Wet");
  } else if (soilMoistureValue1 < AirValue) {
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

    /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  /* Display the results (light is measured in lux) */
  if (event.light)
  {
    Serial.print(event.light); Serial.println(" lux");
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    Serial.println("Sensor overload");


  }

  //co2
  if (mySensor.readMeasurement()) // readMeasurement will return true when fresh data is available
  {
    Serial.println();

    Serial.print(F("CO2(ppm):"));
    Serial.print(mySensor.getCO2());

    Serial.print(F("\tTemperature(C):"));
    Serial.print(mySensor.getTemperature(), 1);

    Serial.print(F("\tHumidity(%RH):"));
    Serial.print(mySensor.getHumidity(), 1);

    Serial.println();
  }
  else
    Serial.print(F("."));






  delay(2000);  // Delay to prevent flooding the serial output
}
