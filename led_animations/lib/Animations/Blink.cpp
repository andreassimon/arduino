#include <Adafruit_NeoPixel.h>

namespace Animations {
  const byte OFF = 0,
        ON  = 1;

  class Blink {
    Adafruit_NeoPixel* strip;
    unsigned int firstPixel, lastPixel;
    byte state;
    uint32_t color, firstColor, secondColor;

    unsigned long lastChange;

    public:
    Blink(Adafruit_NeoPixel* neoStrip, unsigned int first, unsigned int last, uint32_t firstColor, uint32_t secondColor) {
      strip = neoStrip;
      firstPixel = first;
      lastPixel = last;
      state = OFF;
      lastChange = millis();
      (*this).firstColor = firstColor;
      (*this).secondColor = secondColor;
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
          color = firstColor;
        } else {
          color = secondColor;
        }

        for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
          (*strip).setPixelColor(pixel, color);
        }
        (*strip).show();
      }
    }
  };
}
