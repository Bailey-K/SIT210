#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

//Pins
#define PIR_PIN 2
#define SWITCH_PIN 3
#define PORCH_LED 6
#define HALL_LED 5

//Timing
unsigned long porchStart = 0;
unsigned long hallStart = 0;

const unsigned long PORCH_TIME = 30000; // 30 sec
const unsigned long HALL_TIME = 60000;  // 60 sec

//Interrupt flags
volatile bool motionFlag = false;
volatile bool switchFlag = false;

//System states
bool manualOverride = false;
bool motionActive = false;

const float LIGHT_THRESHOLD = 50.0;

//Interupts
void PIR_ISR() {
  if (!motionActive) {
    motionFlag = true;
  }
}

void SWITCH_ISR() {
  switchFlag = true;
}

//setup
void setup() {
  Serial.begin(9600);
  delay(2000);

  Wire.begin();
  lightMeter.begin();

  pinMode(PIR_PIN, INPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  pinMode(PORCH_LED, OUTPUT);
  pinMode(HALL_LED, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIR_PIN), PIR_ISR, RISING);
  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), SWITCH_ISR, CHANGE);

  Serial.println("System Ready");

  if (digitalRead(SWITCH_PIN) == LOW) { //Incase the switch is already activated on startup
    Serial.println("Startup: Switch detected as ON");
    switchFlag = true;
  }
}

//main loop
void loop() {
  unsigned long now = millis();

  if (motionFlag) { //motion detected
    motionFlag = false;
    motionActive = true;

    if (!manualOverride) { //if there is no manual override
      float lightLevel = lightMeter.readLightLevel();

      Serial.print("Motion detected | Light: ");
      Serial.println(lightLevel);

      if (lightLevel < LIGHT_THRESHOLD) {
        motionActive = true;        
        Serial.println("Dark -> Lights ON");

        digitalWrite(PORCH_LED, HIGH);
        digitalWrite(HALL_LED, HIGH);

        porchStart = now;
        hallStart = now;
      }
      else {
        motionActive = false;
      }
    }
    else {
      motionActive = false;
    }
  }

  //Switch
  if (switchFlag) {
    switchFlag = false;

    if (digitalRead(SWITCH_PIN) == LOW) { //Lights on with switch
      if (!motionActive) {
      Serial.println("Switch Activated -> Lights ON");

      digitalWrite(PORCH_LED, HIGH);
      digitalWrite(HALL_LED, HIGH);
      }
      else { //manual override over motion
      Serial.println("Manual Override Activated");
      manualOverride = true;
      }
    } 
    else if (digitalRead(SWITCH_PIN) == HIGH && !motionActive) { //lights off with switch
      Serial.println("Switch Deactivated -> Lights OFF");

      digitalWrite(PORCH_LED, LOW);
      digitalWrite(HALL_LED, LOW);
    }
    else {//manual override off over motion
      Serial.println("Manual Override Deactivated");
      manualOverride = false;
    }
  }

  //Timers
  if (!manualOverride) {

    if (digitalRead(PORCH_LED) == HIGH && now - porchStart >= PORCH_TIME) { //Porch LED on for 30 secs
      digitalWrite(PORCH_LED, LOW);
      Serial.println("Porch OFF");
    }

    if (digitalRead(HALL_LED) == HIGH && now - hallStart >= HALL_TIME) { //Hallway LED on for 60 secs
      digitalWrite(HALL_LED, LOW);
      Serial.println("Hall OFF");

      motionActive = false; //Full motion cycle complete
    }
  }
}