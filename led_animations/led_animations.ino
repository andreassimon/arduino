#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUM_PIXELS 40

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Hello world!");
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
}

// vim:ft=cpp
