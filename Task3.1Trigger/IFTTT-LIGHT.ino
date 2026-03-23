#include <WiFiNINA.h>
#include <BH1750.h>
#include <Wire.h>
#include "secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiClient client;
BH1750 lightMeter;

char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME   = "/trigger/sketch_received/with/key/btCDKb7xvQ0bXKzSR13WOu"; // change your EVENT-NAME and YOUR-KEY
String queryString = "?value1=57&value2=25";

void setup() {
  WiFi.begin(ssid, pass);
  Serial.begin(9600);
  
  Wire.begin();
  lightMeter.begin();
}

void loop() {
  float lux = lightMeter.readLightLevel(); //get light data
  
  //trigger based on light level (> 500 lux)
  if (lux > 500) { 
    String queryString = "?value1=" + String(lux); //use sensor data

    if (client.connect(HOST_NAME, 80)) {
      client.println("GET " + PATH_NAME + queryString + " HTTP/1.1");
      client.println("Host: " + String(HOST_NAME));
      client.println("Connection: close");
      client.println(); 

      while (client.connected() || client.available()) {
        if (client.available()) {
          client.read();
        }
      }
      client.stop();
      Serial.println("Notification Sent!");
      delay(60000); //to prevent notification spam
    }
  }
}