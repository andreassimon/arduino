#include <Adafruit_NeoPixel.h>
#include "Animation.cpp"

namespace Animations {

  const byte KR_PAUSING = 0,
        MOVE_RIGHT = 1,
        MOVE_LEFT  = 2;

  class KnightRider : public Animation {

    Adafruit_NeoPixel* strip;
    unsigned int firstPixel, lastPixel;
    byte state;
    uint32_t darkColor, brightColor;
    unsigned long lastMove, movePeriod, pauseStart, pauseTime;
    unsigned int leftPixel;

    public:
    KnightRider(Adafruit_NeoPixel* strip, unsigned int first, unsigned int last, uint32_t brightColor, uint32_t darkColor) {
      (*this).strip = strip;
      firstPixel = first;
      lastPixel = last;
      state = MOVE_RIGHT;
      leftPixel = first;
      movePeriod = 30ul;
      pauseTime = 1000ul;

      (*this).brightColor = brightColor;
      (*this).darkColor = darkColor;
    }

    void update() {
      switch(state) {
        case KR_PAUSING:
          if(millis() - pauseStart > pauseTime) {
            leftPixel = firstPixel-3;
            state = MOVE_RIGHT;
          }
          break;
        case MOVE_RIGHT:
          if(millis() - lastMove > movePeriod) {
            lastMove = millis();
            leftPixel++;
            if(leftPixel > lastPixel) {
              state = MOVE_LEFT;
            }
            for(unsigned int pixel=firstPixel; pixel<=lastPixel; pixel++) {
              if(leftPixel<=pixel && pixel<=(leftPixel+3)) {
                (*strip).setPixelColor(pixel, brightColor);
              } else {
                (*strip).setPixelColor(pixel, darkColor);
              }
            }
          }
          break;
        case MOVE_LEFT:
          if(millis() - lastMove > movePeriod) {
            lastMove = millis();
            leftPixel--;
            if(leftPixel < firstPixel-3) {
              pauseStart = millis();
              state = KR_PAUSING;
            }
            for(unsigned int pixel=firstPixel; pixel<=lastPixel; pixel++) {
              if(leftPixel<=pixel && pixel<=(leftPixel+3)) {
                (*strip).setPixelColor(pixel, brightColor);
              } else {
                (*strip).setPixelColor(pixel, darkColor);
              }
            }
          }
          break;
      }
      (*strip).show();
    }
  };

}
