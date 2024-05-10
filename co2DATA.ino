/*
  Reading CO2, humidity and temperature from the SCD4x
  By: Paul Clark
  Based on earlier code by: Nathan Seidle
  SparkFun Electronics
  Date: June 3rd, 2021
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/18365

  This example prints the current CO2 level, relative humidity, and temperature in C.

  Hardware Connections:
  Attach RedBoard to computer using a USB cable.
  Connect SCD40/41 to RedBoard using Qwiic cable.
  Open Serial Monitor at 115200 baud.
*/

// Libraries
#include "FS.h"                     // Library for saving data to SD card
#include "SD.h"                     // Library for saving data to SD card
#include "SPI.h"                    // Library for saving data to SD card
int dataTime = 10000;    //10 secs
int dataCount;                      // Counter for saved data points
int dataInterval = 1000; //1 sec
int lastDataTime;
int timeAtCollection;
#include <Wire.h>

#include "SparkFun_SCD4x_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD4x
SCD4x mySensor;

// SD Card
String filename = "/CtrlData.csv";  // Filename for data file
File datafile;                      // Create instance of data file handler object
int dataPinHold;                    // Holds last state of data pin to determine state change 

// Timing Variables
const long timeStep = 1000;         // Time in ms between readings
long senseNext;                     // Time in ms for next read
long startTime;                     // Time stamp in ms at initialization for time offset


void setup()
{
  Serial.begin(115200);
  Serial.println(F("SCD4x Example"));
  Wire.begin();

  //mySensor.enableDebugging(); // Uncomment this line to get helpful debug messages on Serial

  //.begin will start periodic measurements for us (see the later examples for details on how to override this)
  if (mySensor.begin() == false)
  {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1)
      ;
  }
  
  startTime = millis();
  lastDataTime = millis();
  senseNext = 0;
  dataPinHold = 0;
  dataCount = 0;

  if(!SD.begin()){
      Serial.println("Card Mount Failed");
      return;
    }
    datafile = SD.open(filename, FILE_WRITE);

  //The SCD4x has data ready every five seconds
}

void loop()
  
{
  long currTime = millis() - startTime; 
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
  // RECORD DATA TO SD CARD
    
    
    // Write header on reading start
    
    dataCount = 0;          
    
    
    // Write data
    timeAtCollection = currTime/1000.0;
    Serial.print("currTime-lastdataTime"); Serial.println(currTime-lastDataTime); 
    if(currTime-lastDataTime>= dataInterval){
      Serial.println("write data");
      datafile.print(timeAtCollection, 1);
      datafile.print("CO2(ppm):");
      datafile.print(mySensor.getCO2());

      datafile.print("Temperature(C):");
      datafile.print(mySensor.getTemperature(), 1);

      datafile.print("Humidity(%RH):");
      datafile.print(mySensor.getHumidity(), 1);
      datafile.println();
      lastDataTime = currTime;

    }
    
    
    
    // Close file on reading stop
    Serial.print("currTime"); Serial.print(currTime); Serial.print(" Data time"); Serial.println(dataTime);
    if (currTime>= dataTime){
      datafile.close();
      SD.end();
      Serial.println("Ending data collecting process");

    }
    

        
     

  delay(2000);
}
