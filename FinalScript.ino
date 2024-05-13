//Light Sensor
const int lightLowerBound = 100; //lower bound for light; if lower than this, turn grow light on
const int lightUpperBound = 200; //upper bound for light; if higher than this, turn grow light off

//Soil Sensor
int soilMoistureValue1  = 0;  // Variable to store the moisture value
int soilMoistureValue2 = 0;
int soilMoistureValue3 = 0;

int soilPercent1 = 0; //Variables for soil moisture percentage
int soilPercent2 = 0;
int soilPercent3 = 0;

//Pins for relays and button
const int relaySoilPin = 16;
const int relayLightPin = 25;

//Variables to remember time
const int pumpRunTime = 5000; //milliseconds; time we want the pump to turn on for
int pumpOn = 0; //0 means pump is off
unsigned long lastPumpTimeActive = 0; //milliseconds; records the last time the pump was on
const unsigned long pumpInterval = 18*3.6*pow(10, 6); //18 hours; the interval to check if we need to water plant

//Variables for collecting data
int dataTime = 86400000;    // milliseconds; the total amount of time we want to collect data for
int dataInterval = 60000; //milliseconds; the interval we want to collect data at
int lastDataTime = 0;// milliseconds; records the last time the data was collected
int timeAtCollection;// seconds; the time stamp of the data that was collected



#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

WiFiServer server(80);
//Light sensor libraries
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
//Co2, temperature, humidity sensor library
#include "SparkFun_SCD4x_Arduino_Library.h"

#include "FS.h"                     // Library for saving data to SD card
#include "SD.h"                     // Library for saving data to SD card
#include "SPI.h"                    // Library for saving data to SD card

const char *ssid = "yourAP";
const char *password = "yourPassword";

SCD4x mySensor; //co2
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); //initialize light sensor 1
Adafruit_TSL2561_Unified tsl2 = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 67899); //initialize light sensor 2

// SD Card
String filename = "/CtrlData.csv";  // Filename for data file
File datafile;                      // Create instance of data file handler object

// Timing Variables

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


  tsl2.getSensor(&sensor);
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
  tsl2.enableAutoRange(true);  
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  tsl2.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
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
  pinMode(relayLightPin, OUTPUT); //sets up the pin for relay connected to grow lights
  pinMode(relaySoilPin, OUTPUT); //sets up the pin for relay connected to soil stuff
  Serial.println(F("SCD4x"));
  Wire.begin();
  Serial.println(F("System initialization... "));
  Serial.println("Configuring access point...");


  // Initialize timing variables
  
  startTime = millis();


  // Initialize the SCD41 sensor
  Serial.println("Starting sensors test...");
  if (!mySensor.begin()){
    Serial.println("Fail scd4x sensor");
    
  } else {
    Serial.println("SCD4x sensor initialized correctly"); 
  }
  //Light sensor 
    /* Display some basic information on this sensor */
  displaySensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();

  //WIFI 
  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
  //Initializing the SD card
  if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
      }
  datafile = SD.open(filename, FILE_WRITE);
  Serial.println("Recording Started");
  
  /* We're ready to go! */
  Serial.println("Setup complete, entering main loop");
}




void loop() {


  // pump timer
  unsigned long currentMillisPump = millis();


  
  long currTime = millis() - startTime;   
 
  soilMoistureValue1 = analogRead(A0);  // read soil moisture value from moisture sensor 1
  soilMoistureValue2 = analogRead(A3); // read soil moisture value from moisture sensor 2
  soilMoistureValue3 = analogRead(A6); // read soil moisture value from moisture sensor 2

  //converts the soil moisture values to percentage
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

  Serial.print("Average Soil Percent for 3 sensors: ");
  Serial.println(avgSoilPercent); Serial.println("%"); Serial.println("################");



  

  
  Serial.print("current time for pump"); Serial.println(currentMillisPump - lastPumpTimeActive);
  Serial.print("pump interval"); Serial.println(pumpInterval);
  // Check if it's time to activate the pump
  if (currentMillisPump - lastPumpTimeActive >= pumpInterval) {

    // Check soil moisture conditions to decide whether to turn on or off the pump
    if ((soilPercent1 < 20 && soilPercent2 < 20) && soilPercent3<20) {
        digitalWrite(relaySoilPin, HIGH); // Soil is dry, turn on the pump
        Serial.println("Pump: ON (Soil Dry)");
        pumpOn = 1;
    }
    

    lastPumpTimeActive = currentMillisPump; // Reset the timer
  }
  //if the pump is on and its been on for longer than pumpRunTime milliseconds
  if(pumpOn && currentMillisPump - lastPumpTimeActive>= pumpRunTime){
    digitalWrite(relaySoilPin, LOW); // Soil is moist enough, turn off the pump
    Serial.println("Pump: OFF (Soil Moist)");
    pumpOn = 0;

  }
  // a secondary check
  // one of the soil sensor is detecting a soil moisture level of over 20%, turn the pump off
  if (soilPercent1 > 20 || soilPercent2 > 20 || soilPercent3 > 20) {
    digitalWrite(relaySoilPin, LOW); 
    Serial.println("Pump: OFF (Soil Moist)");
    pumpOn = 0;
  }

  



 



  //light sensor 
    /* Get a new sensor event */ 
  sensors_event_t event;
  sensors_event_t event2;
  tsl.getEvent(&event);
  tsl2.getEvent(&event2);
 
  /* Display the results (light is measured in lux) */
  if (event.light && event2.light)
  {
    Serial.print("Light Sensor 1:");Serial.print(event.light); Serial.print(" lux");
    Serial.print("Light Sensor 2:");Serial.print(event2.light); Serial.println(" lux");
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    if (event.light){
      Serial.println("Sensor 1 overload");

    }
    if (event2.light){
      Serial.println("Sensor 2 overload");

    }


  }

  
  
 
  /* Display the results (light is measured in lux) */
  
  //If the light is lower than 100 lux, turn grow lights on
  if(event.light <= lightLowerBound){
    Serial.print("light:"); Serial.print(event.light); Serial.println(" lux");

    Serial.print("turning light on");
    digitalWrite(relayLightPin, HIGH);
  } else if (event.light >= lightUpperBound){ //if light is higher than 200 lux, turn grow lights off
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
  else{
    Serial.println("Co2 sensor Fail");
  }

  // RECORD DATA TO SD CARD
    
  timeAtCollection = currTime/1000.0;
  Serial.print("Data recording interval time"); Serial.println(currTime-lastDataTime); 
  Serial.print("Data interval length");Serial.println(dataInterval);
  if ((currTime-lastDataTime >=dataInterval) ) {
    
    Serial.print("saving data");
    datafile.print(timeAtCollection, 1);
    datafile.print("Soil Sensor 1: ");datafile.print(soilPercent1); datafile.print(" %");
    datafile.print("Soil Sensor 2: ");datafile.print(soilPercent2); datafile.print(" %");
    datafile.print("Soil Sensor 3: ");datafile.print(soilPercent3); datafile.print(" %");
    
    datafile.print("LIGHT(LUX):");
    datafile.print(event.light);

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
  Serial.print("Time"); Serial.print(currTime); Serial.print("Collect data for "); Serial.println(dataTime);
  if (currTime>=dataTime) {
    datafile.close();
    SD.end();
    Serial.println("Recording Ended");
    
  }
  WiFiClient client = server.available(); 

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<!DOCTYPE html>");
            client.println("<html>");
            client.println("<head>");
            client.println("<title style='color:rgb(255, 255, 255)'>Live Sensor Data</title>");
            client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>"); // Add viewport meta tag for responsiveness
            client.println("<style>");
            client.println("#customers {font-family: Arial, Helvetica, sans-serif; border-collapse: collapse; width: 100%;}");
            client.println("#customers td, #customers th {border: 3px solid rgb(255, 255, 255); padding: 8px;}");
            client.println("#customers tr:nth-child(even){background-color: rgb(0, 0, 0);}");
            client.println("#customers tr:hover {background-color: rgb(0, 0, 0);}");
            client.println("#customers th {padding-top: 12px; padding-bottom: 12px; text-align: left; background-color: #349631; color: rgb(255, 255, 255);}");
            client.println("</style>");
            client.println("</head>");
            client.println("<body style='font-family: Arial, sans-serif; margin: 0; padding: 0; background-color:rgb(0, 0, 0)'>"); // Set margin and padding to 0 for the body
            client.println("<h1 style='color: #333; text-align: center;'>Live Sensor Data</h1>"); // Center align heading

            client.println("<body style='margin: 0; background-color:rgb(0, 0, 0); overflow-x: hidden; overflow-y: overlay'>");
            client.println("<div style='padding-top: 5px; display: flex; justify-content: center; align-items: center;'>");

            client.println("</div>");
            client.println("<div style='display:flex; width: 100vw; height: 100vh;'>");
            client.println("<div id='left' style='font-family:Ariel, Arial, Helvetica, sans-serif; color: rgb(255, 255, 255); width:50%; height:50vh; display: flex; flex-direction: column; justify-content: center; align-items: center;'>");
            client.println("<div>");
            client.println("<h2>Soil Moisture Levels Table Summary</h2>");
            client.println("</div>");
            client.println("<div>");
            client.println("<table id='customers'>");
            client.println("<tr>");
            client.println("<th style='padding-left: 20px; padding-right: 20px;''> Sensor Number</th>");
            client.println("<th style='padding-left: 20px; padding-right: 20px;'> Moisture Percentage % </th>");
            client.println("</tr>");
            client.println("<tr>");
            client.println("<td>1</td>");
            client.println("<td>##</td>");
            client.println("</tr>");
            client.println("<tr>");
            client.println("<td style'background-color: rgb(0, 0, 0);'>2</td>");
            client.println("<td style='background-color:rgb(1, 1, 1)'>##</td>");
            client.println("</tr>");
            client.println("<tr>");
            client.println("<td>3</td>");
            client.println("<td>##</td>");
            client.println("</tr>");
            client.println("</table>");
            client.println("</div>");
            client.println("</div>");
            client.println("<div id='right' style='font-family:Ariel, Arial, Helvetica, sans-serif; color:rgb(255, 255, 255); width: 50%; height: 70vh; display: flex; flex-direction: column; justify-content: center; align-items: center; padding-right: 70px;'>");
            client.println("<div>");
            client.println("<h2>Additional Data Measured</h2>");
            client.println("</div>");
            client.println("<div style='display: flex; flex-direction: row'>");
            client.println("<h4>Light Status:##</h4>");
            client.println("</div>");
            client.println("<div style='display: flex; flex-direction: row'>");
            client.println("<h4>CO2 Level:##</h4>");
            client.println("</div>");
            
            client.println("<div style='display: flex; flex-direction: row'>");
            client.println("<h4>Temperature Reading:##</h4>");
            client.println("</div>");
            client.println("<div style='display: flex; flex-direction: row'>");
            client.println("<img src='/Users/yujiechen/Downloads/Webpage/3143215.png' width='50' height='50'> ");
            client.println("<h4>Humidity Level:##</h4>");
            client.println("<div id='sensorData'></div>");
            client.println("</div>");
            
            client.println("</div>");
            client.println("</div>");
            client.println("</body>");
            client.println("</html>");

              


            // the content of the HTTP response follows the header:
            if (event.light){
              client.print(event.light); client.println(" lux");
            }
            else
            {
              /* If event.light = 0 lux the sensor is probably saturated
                and no reliable data could be generated! */
              client.println("Sensor overload");
            }

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }


      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }


  delay(2000);  // Delay to prevent flooding the serial output
}