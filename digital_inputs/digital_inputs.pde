/*
Adafruit Arduino - Lesson 6. Inputs
*/

int ledAPin = 5;
int ledBPin = 6;
int ledCPin = 7;
int buttonApin = 9;
int buttonBpin = 8;

byte leds = 0;

void setup()
{
  pinMode(ledAPin, OUTPUT);
  pinMode(ledBPin, OUTPUT);
  pinMode(ledCPin, OUTPUT);
  pinMode(buttonApin, INPUT_PULLUP);
  pinMode(buttonBpin, INPUT_PULLUP);
}

void loop()
{
  digitalWrite(ledCPin, LOW);
  digitalWrite(ledAPin, HIGH);
  delay(500);

  digitalWrite(ledAPin, LOW);
  digitalWrite(ledBPin, HIGH);
  delay(500);

  digitalWrite(ledBPin, LOW);
  digitalWrite(ledCPin, HIGH);
  delay(500);

//  if (digitalRead(buttonApin) == LOW)
//  {
//    digitalWrite(ledAPin, HIGH);
//    digitalWrite(ledBPin, LOW);
//    digitalWrite(ledCPin, LOW);
//  }
//  if (digitalRead(buttonBpin) == LOW)
//  {
//    digitalWrite(ledAPin, LOW);
//    digitalWrite(ledBPin, HIGH);
//    digitalWrite(ledCPin, HIGH);
//  }
}
