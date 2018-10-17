#include <OLEDDisplay.h>
#include <OLEDDisplayFonts.h>
#include <OLEDDisplayUi.h>
#include <SH1106.h>
#include <SH1106Brzo.h>
#include <SH1106Spi.h>
#include <SH1106Wire.h>
#include <SSD1306.h>
#include <SSD1306Brzo.h>
#include <SSD1306Spi.h>
#include <SSD1306Wire.h>


/*
 * NativeSdk by Simon Peter
 * Access functionality from the Espressif ESP8266 SDK
 * This example code is in the public domain
 * 
 * This is for advanced users.
 * Note that this makes your code dependent on the ESP8266, which is generally
 * a bad idea. So you should try to use esp8266/Arduino functionality
 * where possible instead, in order to abstract away the hardware dependency.
 */


/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
extern "C" {
#include "user_interface.h"
}

// Initialize the OLED display using brzo_i2c
// D3 -> SDA
// D5 -> SCL
SSD1306Brzo display(0x3C, 4, 5);
// or
// SH1106Brzo  display(0x3c, D3, D5);

/* Set these to your desired credentials. */
const char *ssid = "ESPap";
const char *password = "thereisnospoon";
bool IsConfigured = false;

ESP8266WebServer server(80);


void handleBody() {
   if (server.hasArg("ssid")== true){ //Check if body received
        IsConfigured = true; 
        server.send(200, "text/html", "<h1>Configuration OK</h1>");
   }
   else
   {
       server.send(200, "text/html", "<h1>Configuration NOT Accepted</h1>");
   }
}

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() { 
  if(!IsConfigured)
  {
    server.send(200, "text/html", "<form action=\"/action_page.php\" method=\"post\">SSID:<br> <input type=\"text\" name=\"ssid\" value=\"Enter\"><br>"
      "Password:<br>  <input type=\"text\" name=\"password\" value=\"Mouse\"><br><br>  <input type=\"submit\" value=\"Submit\"></form>");
  }
  else
  {
      if(digitalRead(9))
      {
         server.send(200, "text/html", "<h1>Input off</h1>");
         digitalWrite(15, LOW);  
      }
      else
      {
         server.send(200, "text/html", "<h1>Input ON</h1>");
         digitalWrite(15, HIGH);  
      } 
  }
}

void setup() {
  delay(1000);
  pinMode(15, OUTPUT);
  pinMode(9, INPUT_PULLUP);
  //Serial.begin(115200);
  //Serial.println();
  //Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid/*, password*/);

  IPAddress myIP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(myIP);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/action_page.php",HTTP_POST, handleBody); //Associate the handler function to the path
  server.begin();
  //Serial.println("HTTP server started");
  display.init();
  display.flipScreenVertically();
  display.setContrast(255);
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth()/2, display.getHeight()/2 - 10, "Solar Power");
  display.display();
}

void loop() {
  server.handleClient();
  
  // Call Espressif SDK functionality - wrapped in ifdef so that it still
  // compiles on other platforms
  //Serial.print("wifi_station_get_hostname: ");
  //Serial.println(wifi_station_get_hostname());
   //display.clear();
   // display.setFont(ArialMT_Plain_10);
   // display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
   // display.drawString(display.getWidth()/2, display.getHeight()/2, "Restart");
   // display.display();
  //delay(1000);
}
