
/* Original Code written by: Johann Brochier
* Meteo en direct via l'API d'Open Weather Map sur WIFI KIT 32 de Heltec
* Auteur : Johann Brochier
* 
* Open Weather Map API data for Heltec WiFi Kit V2
* This revised version created March 2023 by GC_Marvin
* Heltec WiFi Kit 32 / 115200
*/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "images.h"
#include "heltec.h"

void displayIcon(String sky); //declare function
void signalBars ();           //declare function

const char* ssid = "your_ssid";                // SSID Wifi <<<< EDIT <<<<
const char* password =  "your_ssid_password";   // SSID WiFi Password <<<< EDIT <<<<
String town = "your_town_city";                   // EDIT <<<<  your city/town/location name
String Country = "your_country_code";                      // EDIT <<<< your country code
const String endpoint = "https://api.openweathermap.org/data/2.5/weather?q="+town+","+Country+"&units=metric&APPID="; // URL OWM API
const String key = "your_OWM_API_Key"; // OWM API Key     <<<< EDIT <<<<  your Open Weather Map API Key 

void WiFiEvent(WiFiEvent_t event, system_event_info_t info){
  switch(event){
    case SYSTEM_EVENT_STA_START:
      Serial.println("Mode station");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("Connected to :" + String(WiFi.SSID()));
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("Disconnected, waiting to reconnect");
      WiFi.reconnect();
      break;
      default:
      break;
  }
}
 
void wifiConnect() {
  Serial.println("void wifi connect begin");
  WiFi.begin(ssid, password);
  byte count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10) {
    delay(500);
    count++;
    Heltec.display->setBrightness(50); //screen brightness level - 0 is really dim but visible, 255 is max bright
    Heltec.display -> drawString(2, 5, "Wi-Fi connecting...");
    Heltec.display -> display();
  }
  Heltec.display -> clear();
  if(WiFi.status() == WL_CONNECTED)
    {
      Heltec.display -> drawString(2, 5, "Wifi ... OK");
      Heltec.display -> drawString(2, 20, "connected to ssid:");
      Heltec.display -> drawString(2, 33, ssid);
      Heltec.display -> drawString(2, 53, WiFi.macAddress()); // mac address shown - might need to know it for another project
      Heltec.display -> display();
      Serial.println("wifi connected");
    } else {
      Heltec.display -> clear();
      Heltec.display -> drawString(2, 5, "Wifi connection ... Error");
      Heltec.display -> display();
      Serial.println("wifi error");
    //while(1);
    }
      delay(1000); //displays for one second
      Heltec.display -> clear();
}
void displayWeather(String payload){
        Heltec.display -> clear(); // clear the display
        StaticJsonDocument<2048> doc; 
        auto error = deserializeJson(doc, payload); // start parsing JSON
        if (error) {
            Serial.print(F("deserializeJson() Error code : "));
            Serial.println(error.c_str());
            return;
        } else {
            const char* location = doc["name"];     // automatically inserts the"name" of city/town from data
            Heltec.display->setBrightness(50);     //screen brightness level - 0 is really dim but visible, 255 is max bright
            Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
            Heltec.display->setFont(ArialMT_Plain_16);
            Heltec.display -> drawString(0, 0, location);
            //Heltec.display -> setFont(ArialMT_Plain_10);  // comment out lines 88+89
            //Heltec.display -> drawString(0, 0, ("YOURCITY"));   // added this line here so I could make "location" all caps small font (does not auto instert location)
            Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
            Heltec.display->setFont(ArialMT_Plain_10);
            JsonObject data = doc["main"];          // temperature data
            JsonObject weather = doc["weather"][0]; // required for weather icons
            JsonObject wind = doc["wind"];          // for wind data
            JsonObject clouds = doc["clouds"];      // for cloud cover data
            String sky = weather["icon"];
            String desc = weather["description"];
            String cloud = weather["main"];
            float temperature = data["temp"];       //I used float data for more accurate integer - rounds up or down with no decimal place
            float feels = data["feels_like"];
            //float lotemp = data["temp_min"];
            //float hitemp = data["temp_max"];
            int pressure = data["pressure"];
            int humidity = data["humidity"];
            float windspeed = wind["speed"];
            float windgust = wind["gust"];
            int cloudcover = clouds["all"];
            float vsblty = doc["visibility"];                   
            Heltec.display -> drawString(0, 18, "Cc:" + String(cloudcover) + "%");
            Heltec.display -> drawString(40, 18, String(cloud));
            Heltec.display -> drawString(0, 27, "> " + String(desc));
            //Heltec.display -> drawString(0, 18,  "wchill:  " + String(feels,0) + "°c");       // '0' = no decimal - windchill or realfeel or feelslike
            //Heltec.display -> drawString(0, 27,  "Hi:  " + String (hitemp,0) + "°c");         // forecast high temp - hourly
            //Heltec.display -> drawString(44, 27, "Lo:  " + String (lotemp,0) + "°c");         //forecast low temp - hourly
            Heltec.display -> drawString(0, 36,  "RHumi:  " + (String)humidity + "%");        // relative humidity
            Heltec.display -> drawString(0, 45,  "BPress: " + (String)pressure + " hPa");     //barometric pressure
            Heltec.display -> drawString(0, 54, "Wnd: " + String(windspeed*3.6,0));           //convert m/s to km/h *3.6 - '0' with no decimal
            Heltec.display -> drawString(44, 54, "Gst: " + String(windgust*3.6,0) + " km/h"); //convert m/s to km/h *3.6 - '0' with no decimal
            Heltec.display -> drawHorizontalLine(100,18,28);    // col,row,width - horizontal line under current temp
            Heltec.display -> drawHorizontalLine(107,20,14);    //col,row,width - horizontal line
            Heltec.display -> drawHorizontalLine(100,22,28);    // col,row,width - horizontal line above feels like temp
            Heltec.display -> drawString(123, 28,"®");          // put this ® here to indicate real feel temp - it looks like a C with an R in it :)
            Heltec.display -> drawString(122, 5,"C");           // making more space with 'C' in smaller font
            Heltec.display -> setTextAlignment(TEXT_ALIGN_RIGHT);
            Heltec.display -> drawString(128, 37, "vis:" + String(vsblty *.001,1)+ "km");//*.001 convert m to km
            Heltec.display -> setFont(ArialMT_Plain_16);
            Heltec.display -> drawString(129, 0, String(temperature,0) + "°");  //'0'= no decimal - this places current temp to right of location(town/city)in large font
            Heltec.display -> drawString(129, 23, String(feels,0) + "°");       // windchill or realfeel or feelslike - moved this up here for better visibility
           
            //Heltec.display -> setTextAlignment(TEXT_ALIGN_LEFT); //Moves wxDescription under small place name in larger font
            //Heltec.display -> setFont(ArialMT_Plain_16);
            //Heltec.display -> drawString(0, 12, String(desc));
          
            displayIcon(sky);             // call function weather icon
            signalBars();                 // call function wifi signal quality    
            Heltec.display -> display(); 
        }
}
void displayIcon(String sky) { // cloud conditions icon
  String sk = sky.substring(0,2);
  const char* icon;
  switch (sk.toInt()) {
    case 1:
      icon = one_bits;
      break;
    case 2:
      icon = two_bits;
      break;
    case 3:
      icon = three_bits;
      break;
    case 4:
      icon = four_bits;
      break;
    case 9:
      icon = nine_bits;
      break;
    case 10:
      icon = ten_bits;
      break;
    case 11:
      icon = eleven_bits;
      break;
    case 13:
      icon = thirteen_bits;
      break;
    case 50:
      icon = fifty_bits;
      break;  
    default:
      icon = nothing_bits; // no icon found
      break;
  }
    Heltec.display -> drawXbm(65,0,30,30,(const unsigned char *)icon); 
    // Do not make size larger than 32x32 or it won't work as original was 30x30
    // Weather Icon - 100,15,32,32  (leftline,topline,width,height)
    //Heltec.display -> drawXbm(80,15,30,30,(const unsigned char *)icon);
}
void signalBars () {              // wifi signal strength bars - I altered these paramaters, as they were showing no bars but it was still able to receive data.
      long rssi = WiFi.RSSI();
      int bars;
      if (rssi > -55) { 
        bars = 5;
      } else if (rssi < -55 & rssi > -65) {
        bars = 4;
      } else if (rssi < -70 & rssi > -80) {
        bars = 3;
      } else if (rssi < -80 & rssi > -90) {
        bars = 2;
      } else if (rssi < -90 & rssi > -100) {
        bars = 1;
      } else {
        bars = 0;
      }
      for (int b=0; b <= bars; b++) {
        Heltec.display->drawRect(110,50,18,14); // draw rectangle around the signal strength bars ~ (leftline,topline,width,height)
        Heltec.display->fillRect(109 + (b*3),62 - (b*2),2,b*2); 
        //Heltec.display->fillRect(110 + (b*3),50 - (b*2),2,b*2); 
      }
}
void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);  // Activation of the OLED screen and the serial link
  Heltec.display->setBrightness(0);
  Heltec.display->clear();
  analogSetClockDiv(255); // 1338mS 
  Serial.begin(115200);
  delay(50);  //<<<<<
  wifiConnect(); 
 
}

void loop() {
  bool errorHttp = false; 
  if ((WiFi.status() == WL_CONNECTED)) { // Wifi status check  
    HTTPClient http;
    Serial.println("voidloop http begin");
    http.begin(endpoint + key); // URL vers l'API
    int httpCode = http.GET(); 
    if (httpCode > 0) { // Checking the returned HTTP code
        String payload = http.getString();
        Serial.println("Code http : " + (String)httpCode);
        Serial.println(payload);       
        displayWeather(payload);
        Serial.println("Done . . . next weather update in 30 min");
        http.end(); // release of resources
        Serial.println("END HTTP 1");
        delay (1800000); // / This delay is set to update weather imformation every 1,800,000 Milliseconds (30 Minutes) //900,000 = (15 min) //1,200,000 = (20 min)
    } else {
      Heltec.display -> clear();
      Heltec.display -> setBrightness(50); //screen brightness level - 0 is really dim but visible, 255 is max bright
      Heltec.display -> setTextAlignment(TEXT_ALIGN_CENTER);
      Heltec.display -> setFont(ArialMT_Plain_10);
      Heltec.display -> drawString(64,4,"Reconnecting");
      Heltec.display -> drawString(64,20,"to weather service");
      Heltec.display -> drawString(64,36,"Wait ...");
      Heltec.display -> display();
      Serial.println("Request Error HTTP");
      errorHttp = true;
      delay (5000);        

    Serial.println("END HTTP 2");
    http.end(); // release of resources

    }
   }
  }
