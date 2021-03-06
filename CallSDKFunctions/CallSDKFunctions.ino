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
#include <Ticker.h>  //Ticker Library
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
char ip_address[20];
bool startHotSpot = false;
char counter;
char text_buffer1[10];
char text_buffer2[10];
char text_buffer3[10];
int main_time_seconds = 0;
int pre_time_seconds = 0;
int cool_time_seconds = 0;
bool second_trigger;
Ticker tick_counter;

// config static IP
IPAddress ip(192, 168, 1, 250); // where xx is the desired IP Address
IPAddress gateway(192, 168, 0, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network

ESP8266WebServer server(80);


void tick_routine()
{
  if(pre_time_seconds)
  {
    pre_time_seconds--;
  }
  else if(main_time_seconds)
  {
    main_time_seconds--;
  }
  else if(cool_time_seconds)
  {
    cool_time_seconds--;
  }
  second_trigger = !second_trigger;
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleStart() {
   if (server.hasArg("time")== true){ //Check if time received
        String received_time = server.arg("time");        
        pre_time_seconds = 480;
        cool_time_seconds = 180;
        main_time_seconds = received_time.toInt()*60;
        server.sendHeader("Location", String("/"), true);
        server.send ( 302, "text/plain", "");        
   }
   else
   {
       server.send(200, "text/html", "<h1>NOT Accepted</h1>");
   }
}

void handleManualStart() {
    if(pre_time_seconds)
    {
      pre_time_seconds = 0;
    }
    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");        
}

void handleStop() {
    if(!main_time_seconds)
    {
      cool_time_seconds = 0;
    }
    pre_time_seconds = 0;
    main_time_seconds = 0;   
    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");        
}

void handleBody() {
   if (server.hasArg("ssid")== true){ //Check if body received
        EEPROM.begin(512); 
        String received_SSID = server.arg("ssid");
        String received_password = server.arg("password");
        String received_IP = server.arg("p_address");
        received_SSID.toCharArray(ssid_name, sizeof(ssid_name));
        received_password.toCharArray(ssid_password, sizeof(ssid_password));
        received_IP.toCharArray(ip_address, sizeof(ip_address));
        EEPROM.put(1,ssid_name);
        EEPROM.put(21, ssid_password);
        EEPROM.put(41, ip_address);
        
        IsConfigured = true; 
        EEPROM.commit();
        EEPROM.end(); 
        EEPROM.begin(512); 
        EEPROM.get(1,ssid_name);
        EEPROM.get(21,ssid_password);
        EEPROM.get(41,ip_address);
        EEPROM.end();
        String message = String("<h1>SSID: ") + String(ssid_name) + String("<br>Password: ") + String(ssid_password) +
          String("<br>IP address: ") + String(ip_address) + String("</h1><br> Please Restart the module");
        server.send(200, "text/html", message);
        delay(1000);
        //ESP.restart();
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
    server.send(200, "text/html", "<head><style>html,body {"
      "height:100%;"
      "width:100%;"
      "margin:0;"
      "background: #466368;  background: -webkit-linear-gradient(#648880, #293f50);  background:    -moz-linear-gradient(#648880, #293f50);"
      "}"
      "body , body {"
      "display:flex;"
      "}"
      "form {"
      "margin:auto;"
      "}</style></head><body><br><div align=\"center\"><h1>Solar Power</h1><br><form "
      "action=\"/action_page.php\" method=\"post\"><p><input type=\"text\" name=\"ssid\" placeholder=\"Your SSID\"></p>"
      "<p><input type=\"text\" name=\"password\" placeholder=\"Your password\"></p> "
      "<p>   IP address:<br>  <input type=\"text\" name=\"ip_address\" value=\"192.168.1.250\"></p><p> <input type=\"submit\" value=\"Submit\"></p></form></div></body>");
  }
  else
  {
      if(!(main_time_seconds || pre_time_seconds || cool_time_seconds))
      {
        server.send(200, "text/html", "<head><style>html,body {"
          "height:100%;"
          "width:100%;"
          "margin:0;"
          "background: #466368;  background: -webkit-linear-gradient(#648880, #293f50);  background:    -moz-linear-gradient(#648880, #293f50);"
          "}"
          "body , body {"
          "display:flex;"
          "}"
          "form {"
          "margin:auto;"
          "}</style></head><body><br><div align=\"center\"><h1>Solar Power</h1><br><form action=\"/set_time_page.php\"method=\"post\">"
            "<select name=\"time\">"
            "<option value=\"1\">1</option>"
            "<option value=\"2\">2</option>"
            "<option value=\"3\">3</option>"
            "<option value=\"4\">4</option>"
            "<option value=\"5\">5</option>"
            "<option value=\"6\">6</option>"
            "<option value=\"7\">7</option>"
            "<option value=\"8\">8</option>"
            "<option value=\"9\">9</option>"
            "<option value=\"10\">10</option>"
            "<option value=\"11\">11</option>"
            "<option value=\"12\">12</option>"
            "<option value=\"13\">13</option>"
            "<option value=\"14\">14</option>"
            "<option value=\"15\">15</option>"
            "<option value=\"16\">16</option>"
            "<option value=\"17\">17</option>"
            "<option value=\"18\">18</option>"
            "<option value=\"19\">19</option>"
            "<option value=\"20\">20</option>"
            "</select>"
            "<br><br>"
          "<input type=\"submit\">"
          "</form>"
          "</div></body>");       
      }
      else if (pre_time_seconds)
      {
        server.send(200, "text/html", "<head><meta http-equiv='refresh' content='5' ><style>html,body {"
          "height:100%;"
          "width:100%;"
          "margin:0;"
          "background: #466368;  background: -webkit-linear-gradient(#648880, #293f50);  background:    -moz-linear-gradient(#648880, #293f50);"
          "}"
          "body , body {"
          "display:flex;"
          "}"
          "form {"
          "margin:auto;"
          "}</style></head><body><br><div align=\"center\"><h1>Solar Power</h1><br>"
          "Waiting:" + String(text_buffer1) + "<br><h2>Working:" + String(text_buffer3) + "</h2><br>Cooling:" + String(text_buffer2) + 
          "<br><form action=\"/start_time_page.php\" method=\"post\">"  
          "<input type=\"submit\" value=\"Start\">"
          "</form>"
          "<br><form action=\"/work_time_page.php\" method=\"post\">"  
          "<input type=\"submit\" value=\"Stop\"></p></form></div></body>");
      }
      else
      {
        server.send(200, "text/html", "<head><meta http-equiv='refresh' content='5' ><style>html,body {"
          "height:100%;"
          "width:100%;"
          "margin:0;"
          "background: #466368;  background: -webkit-linear-gradient(#648880, #293f50);  background:    -moz-linear-gradient(#648880, #293f50);"
          "}"
          "body , body {"
          "display:flex;"
          "}"
          "form {"
          "margin:auto;"
          "}</style></head><body><br><div align=\"center\"><h1>Solar Power</h1><br>"
          "Waiting:" + String(text_buffer1) + "<br><h2>Working:" + String(text_buffer3) + "</h2><br>Cooling:" + String(text_buffer2) + 
          "<br><form action=\"/work_time_page.php\" method=\"post\">"  
          "<input type=\"submit\" value=\"Stop\"></p></form></div></body>");
      } 
  }
}

void setup() {
  delay(1000);
  pinMode(15, OUTPUT);
  pinMode(13, INPUT_PULLUP);
  //Serial.begin(115200);
  //Serial.println();
  //Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  EEPROM.begin(512); 
  EEPROM.get(1,ssid_name);
  EEPROM.get(21,ssid_password);
  EEPROM.get(41,ip_address);
  EEPROM.end();
//  if(strlen(ssid_name) == 0)
//  {
//   startHotSpot = true;
//  }
  if(!digitalRead(13))
  {
    startHotSpot = true;
  }
  WiFi.setAutoConnect(false);
  display.init();
  display.flipScreenVertically();
  display.setContrast(255);
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(display.getWidth()/2, 10, "Solar Power");
  if(startHotSpot)
  {
    sprintf(ssid_name, "Solarium_%08x",ESP.getChipId());
    WiFi.softAP(ssid_name/*, password*/);
    IPAddress myIP = WiFi.softAPIP();
    //Serial.print("AP IP address: ");
    //Serial.println(myIP);
   
    display.drawString(display.getWidth()/2, 25, "HotSpot:");
    display.drawString(display.getWidth()/2, 35, ssid_name);
    display.drawString(display.getWidth()/2, 45, myIP.toString());
    display.display();
  }
  else
  {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);    
    //WiFi.begin(ssid_name, ssid_password);
    if(ip.fromString(ip_address))
    {
      WiFi.config(ip, gateway, subnet);
    }
    WiFi.begin(ssid_name, ssid_password);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(1, 20, ssid_name);
    display.drawString(1, 30, ssid_password);
    IsConfigured = true;   
    tick_counter.attach(1,tick_routine);
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/action_page.php",HTTP_POST, handleBody); //Associate the handler function to the path 
  server.on("/set_time_page.php",HTTP_POST, handleStart); 
  server.on("/work_time_page.php",HTTP_POST, handleStop); 
  server.on("/start_time_page.php",HTTP_POST, handleManualStart);  
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
    display.fillRect(0, 40, display.getWidth(), 20);
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
  if( !startHotSpot)
  {
    if(!digitalRead(13))
    {
      pre_time_seconds = 0;
    }
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(20, 50, "IP:");
    display.drawString(40, 50, WiFi.localIP().toString());
    if(second_trigger)
    {
      display.drawString(4, 50, "<>");
    }
    else
    {
      display.drawString(4, 50, "><");
    }
    sprintf(text_buffer1, "%02d:%02d",pre_time_seconds/60, pre_time_seconds%60);
    display.drawString(2, 5, text_buffer1);
    sprintf(text_buffer2, "%02d:%02d",cool_time_seconds/60, cool_time_seconds%60);
    display.drawString(100, 5, text_buffer2);
    display.setFont(ArialMT_Plain_24);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    sprintf(text_buffer3, "%02d:%02d",main_time_seconds/60, main_time_seconds%60);
    display.drawString(display.getWidth()/2, 32, text_buffer3);
    display.display();
    if((!pre_time_seconds) && main_time_seconds)
    {
      digitalWrite(15,1);
    }
    else
    {
      digitalWrite(15,0);
    }
  }
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

