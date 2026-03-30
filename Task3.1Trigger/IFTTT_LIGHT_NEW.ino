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

bool sunlightPresent = false;
float threshold = 500.0;

void setup() {
  Serial.begin(9600);
  setupWiFi();
  setupSensor();
}

void loop() {
  float lux = lightMeter.readLightLevel(); //get/update light data
  checkThreshold(lux);
  delay (5000);
}

void setupWiFi() { //to setup the wifi
    WiFi.begin(ssid, pass);
    Serial.println("WiFi Connected");
}

void setupSensor() { //To setup the sensor and verify its working
  Wire.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750 is working");
  } else {
    Serial.println("BH1750 is not working");
  }
}

float readLightLevel() { //to output the current light data
  float lux = lightMeter.readLightLevel();
  Serial.print("Current Light: ");
  Serial.print (lux);
  Serial.println (" lx");
  return lux;
}

void checkThreshold(float lux) {
  //trigger for sunlight starting
  if (lux > threshold && sunlightPresent == false) {
    sendNotification("Sunlight_Started", lux);
    sunlightPresent = true;
  } 
  //trigger for sunlight ending
  else if (lux <= threshold && sunlightPresent == true) {
    sendNotification("Sunlight_Stopped", lux);
    sunlightPresent = false;
  }
  else {
    readLightLevel(); //intermitent monitoring
  }
}

void sendNotification(String event, float value) {
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
    readLightLevel();
    delay(1000); //to prevent notification spam
  }
}