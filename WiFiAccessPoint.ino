/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
//#define LED_BUILTIN 2   // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED

// Set these to your desired credentials.
const char *ssid = "yourAP";
const char *password = "yourPassword";
int soilMoistureValue1  = 0;  // Variable to store the moisture value
int soilMoistureValue2 = 0;
int soilMoistureValue3 = 0;

int soilPercent1 = 0;

WiFiServer server(80);


Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

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
  //pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}

void loop() {
  sensors_event_t event;
  tsl.getEvent(&event);
  // Read and process soil moisture data
  soilMoistureValue1 = analogRead(A0);  // Read the value from Analog pin 0
  soilMoistureValue2 = analogRead(A3);
  soilMoistureValue3 = analogRead(A6);
  soilPercent1 = exp(-1*(soilMoistureValue1 -3248.5)/449.9);


  Serial.print("Soil Moisture Raw Value for sensor 1: ");
  Serial.println(soilMoistureValue1);  // Display the raw value of soil moisture
  Serial.println(soilPercent1);


 

 




  WiFiClient client = server.available();   // listen for incoming clients

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
            client.println("<title>Live Sensor Data</title>");
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
            client.println("</head>");
            client.println("<body>");
            client.println("<h1>Live Sensor Data</h1>");
            client.println("<div id='sensorData'></div>");
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
}
