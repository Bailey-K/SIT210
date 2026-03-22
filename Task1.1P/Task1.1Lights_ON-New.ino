int button = 2;
int ledP = 3;
int ledH = 4;

unsigned long startTime = 0; //Stores the moment the button was pressed
int state = 0; //Everything starts off 

void setup() //Defines the pin modes
{
  pinMode(button, INPUT_PULLUP);
  pinMode(ledP, OUTPUT);
  pinMode(ledH, OUTPUT);
}

void loop() //Runs each function 
{
  checkButton();
  updateLighting();
}

void checkButton() //Checks button status
{
  //Check if button is pressed
  //Repressing the button after starting will restart the timers
  if (digitalRead(button) == LOW) 
  {
    startTime = millis();
    state = 1;
  }
}

void updateLighting() //Updates the lights
{
  unsigned long elapsed = millis() - startTime;

  switch (state) 
  {
      case 1: //First 30 secs
        digitalWrite(ledP, HIGH);
        digitalWrite(ledH, HIGH);
        if (elapsed >= 30000) state = 2;
        break;

      case 2: //Next 30 secs
        digitalWrite(ledP, LOW);
        digitalWrite(ledH, HIGH);
        if (elapsed >= 60000) state = 0; //Turn back off
        break;

      default: //Off
        digitalWrite(ledP, LOW);
        digitalWrite(ledH, LOW);
        break;
  }
}