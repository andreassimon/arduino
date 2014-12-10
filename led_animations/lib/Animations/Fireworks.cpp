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
