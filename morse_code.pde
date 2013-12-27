/*
  SOS
  
  sends the SOS morse code
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);     
}

void blinkShort() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(200);  
}

void blinkLong() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(600);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(400);  
}

void sendS() {
  blinkShort();
  blinkShort();
  blinkShort();
  delay(400);
}

void sendO() {
  blinkLong();
  blinkLong();
  blinkLong();
  delay(400);
}

// the loop routine runs over and over again forever:
void loop() {
  sendS();
  sendO();
  sendS();  
  
  delay(2000);
}
