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
#include <EEPROM.h>


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

char ssid_name[20];
char ssid_password[20];
bool startHotSpot = true;
char counter;

ESP8266WebServer server(80);

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

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
  EEPROM.begin(512); 
  for(unsigned char i  = 1; i < 20; i++)
  {
    ssid_name[i-1] = EEPROM.read(i);
  }
  if(strlen(ssid_name) > 0)
  {
    for(unsigned char i  = 21; i < 40; i++)
    {
      ssid_password[i-21] = EEPROM.read(i);
      if( ssid_password[i-21] == ' ')
      {
        ssid_password[i-21] = 0; // Space means no password!!!
        break;
      }
    } 
    startHotSpot = true;
  }
  if(digitalRead(9))
  {
    startHotSpot = true;
  }
  
  display.init();
  display.flipScreenVertically();
  display.setContrast(255);
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth()/2, 10, "Solar Power");
  startHotSpot = false;
  if(startHotSpot)
  {
    sprintf(ssid_name, "Solarium_%08x",ESP.getChipId());
    WiFi.softAP(ssid_name/*, password*/);
    IPAddress myIP = WiFi.softAPIP();
    //Serial.print("AP IP address: ");
    //Serial.println(myIP);
   
    display.drawString(display.getWidth()/2, 25, "HotSpot:");
    display.drawString(display.getWidth()/2, 35, ssid_name);
    display.display();
  }
  else
  {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);    
    //WiFi.begin(ssid_name, ssid_password);
    WiFi.begin("LENX");
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(1, 25, "SSID:");
    display.drawString(35, 25, "ssid_name");   
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/action_page.php",HTTP_POST, handleBody); //Associate the handler function to the path  
  server.onNotFound(handleNotFound);           // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.begin();
  //Serial.println("HTTP server started");
 
  //display.drawString(display.getWidth()/2, display.getHeight()/2 - 10, "Solar Power");
 
}

void loop() {
  if((!startHotSpot) && (WiFi.status() != WL_CONNECTED))
  {
    delay(500);          
    display.setColor(BLACK);
    display.fillRect(0, 35, display.getWidth(), 20);
    display.setColor(WHITE);
    counter++;     
    switch(counter)
    {        
      case 1:
        display.drawString(2, 45, "Connecting.");
        break;
      case 2:
        display.drawString(2, 45, "Connecting...");
        break;        
      case 3:             
      default:
        display.drawString(2, 45, "Connecting.....");          
        counter = 0;
        break;
    }
      
    display.display();
    
    return;  
  }  
  display.clear();
  display.drawString(2, 45, WiFi.localIP().toString());
  display.display();
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

