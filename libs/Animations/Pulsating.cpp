#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

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
