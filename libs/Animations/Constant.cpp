#include <Adafruit_NeoPixel.h>
#include "Animation.cpp"

namespace Animations {
  class Constant : public Animation {
    Adafruit_NeoPixel* strip;
    unsigned int firstPixel, lastPixel;
    uint32_t color;
    boolean isSetup;

    public:
    Constant(Adafruit_NeoPixel* neoStrip, unsigned int first, unsigned int last, uint32_t color) {
      strip = neoStrip;
      firstPixel = first;
      lastPixel = last;
      isSetup = false;
      (*this).color = color;
    }

    void update() {
      if(!isSetup) {
        for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
          (*strip).setPixelColor(pixel, color);
        }
        (*strip).show();
        isSetup = true;
      }
    }
  };
}
