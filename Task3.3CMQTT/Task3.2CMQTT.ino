#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include "secrets.h" 

//MQTT config
const char broker[] = "broker.emqx.io"; 
int        port     = 1883;             
const char typeWave[]  = "ES/Wave";    
const char typePat[]   = "ES/Pat";     

//hardware pins
const int trigPin = 2; 
const int echoPin = 3; 
const int hallwayLED = 5;  
const int bathroomLED = 6; 

//logic and timing variables
bool lightsAreOn = false; 
unsigned long lastTriggerTime = 0;
const unsigned long cooldown = 3000;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

void onMqttMessage(int messageSize) {
  String type = mqttClient.messageTopic();
  
  while (mqttClient.available()) { mqttClient.read(); } 

  if (type == typeWave) {
    digitalWrite(hallwayLED, HIGH);
    digitalWrite(bathroomLED, HIGH);
    lightsAreOn = true;
    Serial.println(" - MQTT Received: Turning Lights ON");
  } else if (type == typePat) {
    digitalWrite(hallwayLED, LOW);
    digitalWrite(bathroomLED, LOW);
    lightsAreOn = false;
    Serial.println(" - MQTT Received: Turning Lights OFF");
  }
}

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5); 
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration <= 0) return 999; //catches sensor errors by assigning an impossible distance (999 cm) to any sound pulse that fails to return, preventing random "ghost" triggers.
  return duration * 0.034 / 2; //The formula calculates distance by multiplying the travel time by the speed of sound and dividing by 2 to account for the round trip.
}

void setup() {
  Serial.begin(9600);
  delay(2000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(hallwayLED, OUTPUT);
  pinMode(bathroomLED, OUTPUT);

  connectToNetwork();
}

void connectToNetwork() {
  Serial.print("Connecting to WiFi...");
  while (WiFi.begin(SECRET_SSID, SECRET_PASS) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Serial.print("Connecting to Broker...");
  if (!mqttClient.connect(broker, port)) {
    Serial.print("Failed! Error: ");
    Serial.println(mqttClient.connectError());
    return;
  }
  Serial.println(" Connected!");

  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(typeWave);
  mqttClient.subscribe(typePat);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED || !mqttClient.connected()) {
    Serial.println("Connection Lost. Reconnecting...");
    connectToNetwork();
    return; 
  }

  mqttClient.poll(); 
  long distance = getDistance();
  unsigned long currentTime = millis();

  if (currentTime - lastTriggerTime > cooldown) {
    
    //Triggered only if lights are OFF. Range: 12-25cm
    if (!lightsAreOn && distance >= 12 && distance <= 25) { 
      Serial.print("Wave Valid");
      mqttClient.beginMessage(typeWave);
      mqttClient.print("Bailey");
      mqttClient.endMessage();
      lastTriggerTime = currentTime;
    } 
    
    //Triggered only if lights are ON. Range: 3-8cm
    else if (lightsAreOn && distance >= 3 && distance <= 8) { 
      Serial.print("Pat Valid");
      mqttClient.beginMessage(typePat);
      mqttClient.print("Bailey"); 
      mqttClient.endMessage();
      lastTriggerTime = currentTime;
    }
  }
}