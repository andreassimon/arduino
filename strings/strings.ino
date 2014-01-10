String aString;

void setup() {
  Serial.begin(9600);

  aString = String("");
}

void loop() {
  aString += 'a';

  // if(aString.length() % 100 == 0) {
  if(aString.length()> 400) {
    Serial.println(aString);
    Serial.println(aString.length());
    delay(10);
  }
}

// vim:ft=c
