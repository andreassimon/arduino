#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUM_PIXELS 58

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

const uint32_t BLACK = strip.Color(0, 0, 0);
const uint32_t RED = strip.Color(128, 0, 0);

#include "lib/Animations/Blink.cpp"
#include "lib/Animations/PulsatingRed.cpp"
#include "lib/Animations/Fireworks.cpp"
#include "lib/Animations/KnightRider.cpp"

Animations::Blink blink = Animations::Blink(&strip,  1-1, 15-1, RED, BLACK);

Animations::PulsatingRed pulsating = Animations::PulsatingRed(&strip, 17-1, 17+14-1, 100, 200);

Animations::Fireworks fireworks = Animations::Fireworks(&strip, 17+14+1-1, 17+14+1+14-1);
Animations::KnightRider knightRider = Animations::KnightRider(&strip, 17+14+1+14+1-1, 17+14+1+14+1+14-1, 0xc00000, 0x3c0000);

Animations::Animation* animations[] = {
  &blink,
  &pulsating,
  &fireworks,
  &knightRider
};

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Hello world!");
  strip.begin();

  for(int i=0; i<NUM_PIXELS; i++) {
    strip.setPixelColor(i, 0u);
  }
  strip.show();
  delay(250);
}

void loop() {
  for(int i = 0; i<4; i++) {
    (*animations[i]).update();
  }

  // Knight Rider
}

// vim:ft=cpp
