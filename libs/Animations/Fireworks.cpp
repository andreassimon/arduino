#include <Adafruit_NeoPixel.h>
#include "Animation.cpp"

namespace Animations {

  const byte PAUSING = 0,
        SHOOTING = 1;

  class Fireworks : public Animation {

    Adafruit_NeoPixel* strip;
    unsigned int firstPixel, lastPixel;
    byte state;
    unsigned long lastFlash, lastShade, flashPeriod, shadePeriod, pauseStart, pauseTime;
    uint32_t currentColor;
    unsigned int shootingPixel;

    public:
    Fireworks(Adafruit_NeoPixel* strip, unsigned int first, unsigned int last) {
      (*this).strip = strip;
      firstPixel = first;
      lastPixel = last;
      state = SHOOTING;
      shootingPixel = first;
      flashPeriod = 30;
      shadePeriod = 5;
      pauseTime = 1000ul;
    }

    void update() {
      switch(state) {
        case PAUSING:
          if(millis() - pauseStart > pauseTime) {
            shootingPixel = firstPixel-1;
            state = SHOOTING;
          }
          break;
        case SHOOTING:
          if(millis() - lastFlash > flashPeriod) {
            lastFlash = millis();
            shootingPixel++;
            if(shootingPixel > lastPixel) {
              pauseStart = millis();
              state = PAUSING;
            } else {
              (*strip).setPixelColor(shootingPixel, (*strip).Color(128, 0, 0));
            }
          }
          break;
      }
      if(millis() - lastShade > shadePeriod) {
        lastShade = millis();
        for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
          currentColor = ((*strip).getPixelColor(pixel) - 10) & 0xff0000;
          if(currentColor < 0x080000) currentColor = 0x080000;
          (*strip).setPixelColor(pixel, currentColor);
        }
      }
      (*strip).show();
    }
  };

}
