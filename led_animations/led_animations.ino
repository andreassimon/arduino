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

namespace Animations {
class Pulsating {
  Adafruit_NeoPixel* strip;
  unsigned int firstPixel, lastPixel;
  int colorRed;
  int lowerBoundary,
      upperBoundary;

  unsigned long lastChange, period, delta;

  public:
  Pulsating(Adafruit_NeoPixel* strip, unsigned int first, unsigned int last, int lowerBoundary, int upperBoundary) {
    (*this).strip = strip;
    firstPixel = first;
    lastPixel = last;
    period = 5;
    (*this).lowerBoundary = lowerBoundary;
    (*this).upperBoundary = upperBoundary;
    colorRed = lowerBoundary;
    delta = 1;
    lastChange = millis();
  }

  void update() {
    if(millis() - lastChange > period) {
      lastChange = millis();
      colorRed = colorRed + delta;
      if(colorRed <= lowerBoundary) {
        colorRed = lowerBoundary;
        delta = -delta;
      }
      if(colorRed >= upperBoundary) {
        colorRed = upperBoundary;
        delta = -delta;
      }
      uint32_t color = (*strip).Color(colorRed, 0, 0);
      for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
        (*strip).setPixelColor(pixel, color);
      }
      (*strip).show();
    }
  }
};
}

namespace Animations {
const byte PAUSING = 0,
           SHOOTING = 1;

class Fireworks {

  Adafruit_NeoPixel* strip;
  unsigned int firstPixel, lastPixel;
  byte state;
  int colorRed;
  unsigned long lastChange, period, pauseStart, pauseTime;
  uint32_t color;
  unsigned int shootingPixel;

  public:
  Fireworks(Adafruit_NeoPixel* strip, unsigned int first, unsigned int last) {
    (*this).strip = strip;
    firstPixel = first;
    lastPixel = last;
    state = SHOOTING;
    shootingPixel = first;
    period = 50;
    pauseTime = 2000ul;
    colorRed = 0;
    lastChange = millis();
  }

  void update() {
    color = (*strip).Color(128,0,0);
    switch(state) {
      case PAUSING:
        if(millis() - pauseStart > pauseTime) {
          shootingPixel = firstPixel;
          state = SHOOTING;
        }
        break;
      case SHOOTING:
        if(millis() - lastChange > period) {
          lastChange = millis();
          (*strip).setPixelColor(shootingPixel, BLACK);
          shootingPixel++;
          if(shootingPixel > lastPixel) {
            pauseStart = millis();
            state = PAUSING;
          }

          if(shootingPixel <= lastPixel) {
            (*strip).setPixelColor(shootingPixel, color);
          }
          (*strip).show();
        }
        break;
    }
  }
};
}

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
