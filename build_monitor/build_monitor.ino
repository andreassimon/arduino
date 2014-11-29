#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUM_PIXELS 24
#define BLACK strip.Color( 0,  0,  0)
#define WHITE strip.Color(30, 30, 30)
#define GREEN strip.Color( 0, 30,  0)
#define BLUE  strip.Color( 0,  0, 30)
#define RED   strip.Color(30,  0,  0)
#define WAIT  400

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  colorWipe(RED  , 50);
  colorWipe(GREEN, 50);
  colorWipe(BLUE , 50);
  delay(WAIT);
}

void switchPixelTo(uint16_t pixel, uint32_t color) {
  strip.setPixelColor(pixel, BLACK);
  strip.show();
}

void setPixelRed(uint16_t pixel) {
  switchPixelTo(pixel, RED);
}

void setPixelGreen(uint16_t pixel) {
  switchPixelTo(pixel, GREEN);
}

long randomPixel;
void loop() {
  // Some example procedures showing how to display to the pixels:
  // rainbow(20);
  // rainbowCycle(100);
  setPixelRed(random(NUM_PIXELS));
  delay(WAIT);
  setPixelGreen(random(NUM_PIXELS));
  delay(WAIT);
}

// vim:ft=c
