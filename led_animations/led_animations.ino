#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUM_PIXELS 40

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

const uint32_t BLACK = strip.Color(0, 0, 0);
const uint32_t RED = strip.Color(128, 0, 0);

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

#include "lib/Animations/Blink.cpp"
#include "lib/Animations/Pulsating.cpp"
#include "lib/Animations/Fireworks.cpp"

Animations::Blink blink1 = Animations::Blink(&strip, 35, 36);
Animations::Blink blink2 = Animations::Blink(&strip, 38, 39);

Animations::Pulsating pulsating = Animations::Pulsating(&strip, 23, 32, 100, 200);

Animations::Fireworks fireworks = Animations::Fireworks(&strip, 7, 17);

void loop() {
  blink1.update();
  blink2.update();

  pulsating.update();

  fireworks.update();
  // Knight Rider
}

// vim:ft=cpp
