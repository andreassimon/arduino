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

const byte OFF = 0,
           ON  = 1;

class Animations_Blink {
  Adafruit_NeoPixel* strip;
  unsigned int firstPixel, lastPixel;
  byte state;
  uint32_t color;

  unsigned long lastChange;

  public:
  Animations_Blink(Adafruit_NeoPixel* neoStrip, unsigned int first, unsigned int last) {
    strip = neoStrip;
    firstPixel = first;
    lastPixel = last;
    state = OFF;
    lastChange = millis();
  }

  void update() {
    if(millis() - lastChange > 500) {
      lastChange = millis();
      if(state == OFF) {
        state = ON;
      } else {
        state = OFF;
      }

      if(state == ON) {
        color = RED;
      } else {
        color = BLACK;
      }

      for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
        (*strip).setPixelColor(pixel, color);
      }
      (*strip).show();
    }
  }
};

Animations_Blink blink1 = Animations_Blink(&strip, 35, 36);
Animations_Blink blink2 = Animations_Blink(&strip, 38, 39);

void loop() {
  // Blink
  blink1.update();
  blink2.update();

  // Fireworks
  // Pulsating
  // Knight Rider
}

// vim:ft=cpp
