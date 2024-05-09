// Constants for soil moisture sensor thresholds
const int AirValue = 3207;   // Replace with the maximum dry value (air)
const int WaterValue = 2200; // Replace with the minimum wet value (water)
const int lightLowerBound = 100;
const int lightUpperBound = 200;
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
const int pumpRunTime = 10000; //10 seconds
int pumpOn = 0; //0 means off
// Include the library for the DHT22 sensor

#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

WiFiServer server(80);

#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include "SparkFun_SCD4x_Arduino_Library.h"

#include "FS.h"                     // Library for saving data to SD card
#include "SD.h"                     // Library for saving data to SD card
#include "SPI.h"                    // Library for saving data to SD card

const char *ssid = "yourAP";
const char *password = "yourPassword";

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
  Serial.println("Configuring access point...");


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
  
  /* We're ready to go! */
  Serial.println("Setup complete, entering main loop");
}

unsigned long lastPumpTimeActive = 0;
const unsigned long pumpInterval = 18*3.6*pow(10, 6); //48hours  // 


//const unsigned long pumpInterval = 15000;//15 seconds 




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
  Serial.print("current time interval"); Serial.println(currentMillisPump - lastPumpTimeActive);
  Serial.print("Time interval"); Serial.println(pumpInterval);
  if (currentMillisPump - lastPumpTimeActive >= pumpInterval) {

    // Check soil moisture conditions to decide whether to turn on or off the pump
    if (soilPercent1 < 20) {
        digitalWrite(relaySoilPin, HIGH); // Soil is dry, turn on the pump
        Serial.println("Pump: ON (Soil Dry)");
        pumpOn = 1;
    }
    else if(pumpOn && currentMillisPump - lastPumpTimeActive>= pumpRunTime){
      digitalWrite(relaySoilPin, LOW); // Soil is moist enough, turn off the pump
      Serial.println("Pump: OFF (Soil Moist)");
      pumpOn = 0;

    }
     else if (avgSoilPercent > 20 || soilPercent2 > 20) {
        digitalWrite(relaySoilPin, LOW); // Soil is moist enough, turn off the pump
        Serial.println("Pump: OFF (Soil Moist)");
        pumpOn = 0;
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
    
    datafile.print("LIGHT(LUX):");
    datafile.print(event.light);

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
            client.println("<script>");
            client.println("function updateData() {");
            client.println("  var xhttp = new XMLHttpRequest();");
            client.println("  xhttp.onreadystatechange = function() {");
            client.println("    if (this.readyState == 4 && this.status == 200) {");
            client.println("      document.getElementById('sensorData').innerHTML = this.responseText;");
            client.println("    }");
            client.println("  };");
            client.println("  xhttp.open('GET', '/data', true);");
            client.println("  xhttp.send();");
            client.println("}");
            client.println("setInterval(updateData, 2000);"); // Update data every 5 seconds
            client.println("</script>");
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
            // client.println("Click <a href=\"/H\">here</a> to turn ON the LED.<br>");
            // client.println("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");
            // client.print("Soil Moisture Raw Value for sensor 1: ");
            // //client.println(soilMoistureValue1);  // Display the raw value of soil moisture
            // client.println(soilPercent1);

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

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }


  delay(2000);  // Delay to prevent flooding the serial output
}