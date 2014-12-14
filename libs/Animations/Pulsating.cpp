#ifndef ANI_PULSATING
#define ANI_PULSATING

#include <Adafruit_NeoPixel.h>
#include "Animation.cpp"

namespace Animations {
  class Pulsating: public Animation {
    protected:
    Adafruit_NeoPixel* strip;
    unsigned int firstPixel, lastPixel;
    byte colorBrightness,
         lowerBrightness,
         upperBrightness;

    unsigned long lastChange, period, delta;

    public:
    void update() {
      if(millis() - lastChange > period) {
        lastChange = millis();
        colorBrightness = constrain(colorBrightness + delta, lowerBrightness, upperBrightness);
        if(colorBrightness <= lowerBrightness || colorBrightness >= upperBrightness) {
          delta = -delta;
        }
        uint32_t color = colorFromBrightness(colorBrightness);
        for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
          (*strip).setPixelColor(pixel, color);
        }
        (*strip).show();
      }
    }

    protected:
    virtual uint32_t colorFromBrightness(byte colorBrightness) =0;
  };
}

#endif
