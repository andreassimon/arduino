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

uint32_t color = RED;

const byte OFF = 0,
           ON  = 1;

class Animations_Blink {
  byte state;
  unsigned long lastChange;

  public:
  Animations_Blink() {
    state = OFF;
    lastChange = millis();
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
        color = RED;
      } else {
        color = BLACK;
      }

      for(uint16_t pixel = 0; pixel <= 9; pixel++) {
        Serial.print("Setting pixel ");
        Serial.print(pixel);
        Serial.print(" to ");
        Serial.println(color, HEX);
        strip.setPixelColor(pixel, color);
      }
      strip.show();
    }
  }
};

Animations_Blink animation = Animations_Blink();

void loop() {
  // Blink
  animation.update();


  // Fireworks
  // Pulsating
  // Knight Rider
}

// vim:ft=cpp
