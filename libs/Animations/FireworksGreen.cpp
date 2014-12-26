#include <Adafruit_NeoPixel.h>
#include "Animation.cpp"

namespace Animations {

  class FireworksGreen : public Animation {

    Adafruit_NeoPixel* strip;
    unsigned int firstPixel, lastPixel;
    byte state;
    unsigned long lastFlash, lastShade, flashPeriod, shadePeriod, pauseStart, pauseTime;
    uint32_t currentColor;
    unsigned int shootingPixel;

    public:
    FireworksGreen(Adafruit_NeoPixel* strip, unsigned int first, unsigned int last) {
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
              (*strip).setPixelColor(shootingPixel, (*strip).Color(0, 128, 0));
            }
          }
          break;
      }
      if(millis() - lastShade > shadePeriod) {
        lastShade = millis();
        for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
          currentColor = ((*strip).getPixelColor(pixel) - 10) & 0x00ff00;
          if(currentColor < 0x000800) currentColor = 0x000800;
          (*strip).setPixelColor(pixel, currentColor);
        }
      }
      (*strip).show();
    }
  };

}
