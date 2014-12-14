#include <Adafruit_NeoPixel.h>
#include "Pulsating.cpp"

namespace Animations {
  class PulsatingGreen: public Pulsating {

    public:
    PulsatingGreen(Adafruit_NeoPixel* strip, unsigned int first, unsigned int last, byte lowerBrightness, byte upperBrightness) {
      (*this).strip = strip;
      firstPixel = first;
      lastPixel = last;
      period = 5;
      (*this).lowerBrightness = lowerBrightness;
      (*this).upperBrightness = upperBrightness;
      colorBrightness = lowerBrightness;
      delta = 1;
      lastChange = millis();
    }

    protected:
    uint32_t colorFromBrightness(byte colorBrightness) {
      return (*strip).Color(0, colorBrightness, 0);
    }

  };
}
