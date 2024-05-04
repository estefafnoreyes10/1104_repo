// Constants for soil moisture sensor thresholds
const int AirValue = 3207;   // Replace with the maximum dry value (air)
const int WaterValue = 2200; // Replace with the minimum wet value (water)
const int lightLowerBound = 100;
const int lightUpperBound = 500;
int intervals = (AirValue - WaterValue) / 3;  // Calculate intervals to classify moisture level
int soilMoistureValue1  = 0;  // Variable to store the moisture value
int soilMoistureValue2 = 0;
int soilMoistureValue3 = 0;
int soilMoistureValue4 = 0;
int soilMoistureValue5 = 0;
int soilPercent1 = 0;
int soilPercent2 = 0;
int soilPercent3 = 0;
const int relaySoilPin = 16;
const int relayLightPin = 25;
const int dataPin = 17;             // Input pin for data recording switch

// Include the library for the DHT22 sensor

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include "SparkFun_SCD4x_Arduino_Library.h"

#include "FS.h"                     // Library for saving data to SD card
#include "SD.h"                     // Library for saving data to SD card
#include "SPI.h"                    // Library for saving data to SD card

SCD4x mySensor;
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);


// SD Card
String filename = "/CtrlData.csv";  // Filename for data file
File datafile;                      // Create instance of data file handler object
int dataPinHold;                    // Holds last state of data pin to determine state change 

// Timing Variables
const long timeStep = 1000;         // Time in ms between readings
long senseNext;                     // Time in ms for next read
long startTime;                     // Time stamp in ms at initialization for time offset

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
  //co2
  pinMode(relayLightPin, OUTPUT);
  pinMode(relaySoilPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  Serial.println(F("SCD4x"));
  Wire.begin();
  Serial.println(F("System initialization... "));


  // Initialize timing variables
  delay(500);
  startTime = millis();
  senseNext = 0;
  dataPinHold = 0;


  // logic correction with the false stuff 
  Serial.println("Starting sensors test...");
  if (!mySensor.begin()){
    Serial.println("Fail scd4x sensor");
    
  } else {
    Serial.println("SCD4x sensor initialized correctly"); 
  }
    /* Display some basic information on this sensor */
  displaySensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  /* We're ready to go! */
  Serial.println("Setup complete, entering main loop");
}

unsigned long lastPumpTimeActive = 0;
const unsigned long pumpInterval = 172800000UL; //48hours 




void loop() {


  // pump timer
  unsigned long currentMillisPump = millis();


  // Read and process soil moisture data
  long currTime = millis() - startTime;   
  float dataTime = currTime / 1000.0;
  soilMoistureValue1 = analogRead(A0);  // Read the value from Analog pin 0
  soilMoistureValue2 = analogRead(A3);
  soilMoistureValue3 = analogRead(A6);
  soilPercent1 = exp(-1*(soilMoistureValue1 -3248.5)/449.9);
  soilPercent2 = exp(-1*(soilMoistureValue2 -3248.5)/449.9);
  soilPercent3 = exp(-1*(soilMoistureValue3 -3248.5)/449.9);
  
  int avgSoilPercent = (soilPercent1 +soilPercent2 + soilPercent3)/3;


  Serial.print("Soil Moisture Raw Value for sensor 1: ");
  Serial.println(soilMoistureValue1);  // Display the raw value of soil moisture
  Serial.print(soilPercent1); Serial.println("%"); Serial.println("################");
  Serial.print("Soil Moisture Raw Value for sensor 2: ");
  Serial.println(soilMoistureValue2);  // Display the raw value of soil moisture
  Serial.print(soilPercent2);Serial.println("%");Serial.println("################");
  Serial.print("Soil Moisture Raw Value for sensor 3: ");
  Serial.println(soilMoistureValue3);  // Display the raw value of soil moisture
  Serial.print(soilPercent3);Serial.println("%");Serial.println("################");

  Serial.print("Avergage Soil Percent for 3 sensors: ");
  Serial.println(avgSoilPercent); Serial.println("%"); Serial.println("################");



  // collect from sesnor closest to the pump A0  soilPercent1
  // middle sensor A6 soilPercent3
  //turn off from sesnor farthest from the pump A3 soilPercent2
  // Control the pump based on moisture level

  // Check if it's time to activate the pump
    if (currentMillisPump - lastPumpTimeActive >= pumpInterval) {
        // Check soil moisture conditions to decide whether to turn on or off the pump
        if (soilPercent1 < 20) {
            digitalWrite(relaySoilPin, HIGH); // Soil is dry, turn on the pump
            Serial.println("Pump: ON (Soil Dry)");
        } else if (avgSoilPercent > 20 || soilPercent2 > 20) {
            digitalWrite(relaySoilPin, LOW); // Soil is moist enough, turn off the pump
            Serial.println("Pump: OFF (Soil Moist)");
        }

        lastPumpTimeActive = currentMillisPump; // Reset the timer
    }

  



  // if (soilMoistureValue1 <= WaterValue) {
  //   Serial.println("Status: Water");
  //   digitalWrite(relaySoilPin, LOW); // Soil is very wet, turn off the pump
  // } else if (soilMoistureValue1 <= WaterValue + intervals) {
  //   Serial.println("Status: Very Wet");
  //   Serial.println("Pump: OFF VERY WET");
  //   digitalWrite(relaySoilPin, LOW); // Still wet enough, keep pump off
  // } else if (soilMoistureValue1 <= AirValue - intervals) {
  //   Serial.println("Status: Wet");
  //   Serial.println("Pump: OFF WET");
  //   digitalWrite(relaySoilPin, LOW); // Moist enough, no watering needed
  // } else if (soilMoistureValue1 < AirValue) {
  //   Serial.println("Status: Dry");
  //   digitalWrite(relaySoilPin, HIGH); // Dry, turn on the pump
  //   Serial.println("Pump: ON DRY");
  // } else {
  //   Serial.println("Status: Air");
  //   digitalWrite(relaySoilPin, HIGH); // Very dry, definitely turn on the pump
  //   Serial.println("Pump: ON VERY DRY");
  // }




  //light sensor 
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
    Serial.println("Light sensor overload");


  }
  
  if(event.light <= lightLowerBound){
    Serial.print("light:"); Serial.print(event.light); Serial.println(" lux");

    Serial.print("turning light on");
    digitalWrite(relayLightPin, HIGH);
  } else if (event.light >= lightUpperBound){
    Serial.print("light:"); Serial.print(event.light); Serial.println(" lux");

    Serial.print("turning light off");
    digitalWrite(relayLightPin, LOW);


  }

  //co2
  Serial.println("BEGIN CO2 stuff");
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
    Serial.println("Co2 sensor Fail");

  // RECORD DATA TO SD CARD
    int digPinState = digitalRead(dataPin);
    
    // Write header on reading start
    if (digPinState && !dataPinHold){
      if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
      }
      datafile = SD.open(filename, FILE_WRITE);
      Serial.println("Recording Started");
      
    }
    
    // Write data
    if (digPinState) {
      Serial.println("saving data");
      dataPinHold = digPinState;
      datafile.print(dataTime, 1);
      datafile.print("Soil Sensor 1: ");datafile.print(soilPercent1); datafile.print(" %");
      datafile.print("Soil Sensor 2: ");datafile.print(soilPercent2); datafile.print(" %");
      datafile.print("Soil Sensor 3: ");datafile.print(soilPercent3); datafile.print(" %");
      

      datafile.print("CO2(ppm):");
      datafile.print(mySensor.getCO2());

      datafile.print("Temperature(C):");
      datafile.print(mySensor.getTemperature(), 1);

      datafile.print("Humidity(%RH):");
      datafile.print(mySensor.getHumidity(), 1);

  

 
      datafile.println();
    }
    // Close file on reading stop
    else if (dataPinHold) {
     datafile.close();
     SD.end();
     Serial.println("Recording Ended");
     dataPinHold = digPinState;
    }
  delay(2000);  // Delay to prevent flooding the serial output
}