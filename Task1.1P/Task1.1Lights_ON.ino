int button = 2;
int ledP = 3;
int ledH = 4;

void setup() {
  pinMode(button, INPUT_PULLUP);
  pinMode(ledP, OUTPUT);
  pinMode(ledH, OUTPUT);
}

void loop() {
  if (digitalRead(button) == LOW) {
    digitalWrite(ledP, HIGH);
    digitalWrite(ledH, HIGH);
    delay(30000);
    digitalWrite(ledP, LOW);
    delay(30000);
    digitalWrite(ledH, LOW);
  } else {
    digitalWrite(ledP, LOW);
    digitalWrite(ledH, LOW);
  }
}