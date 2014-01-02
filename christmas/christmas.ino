#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUM_PIXELS 59
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
int pixels[NUM_PIXELS];

unsigned long lastLightTime;
unsigned long lastFadeTime;

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void pixelToBrightness(uint16_t pixel, int brightness) {
//  Serial.print("Setting pixel`");
//  Serial.print(pixel);
//  Serial.print("` to brightness `");
//  Serial.print(brightness);
//  Serial.println("`");
  pixels[pixel] = brightness;
  strip.setPixelColor(pixel, strip.Color(brightness, brightness, brightness));
}

void allPixelsToBrightness(int brightness) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    pixelToBrightness(i, brightness);
  }
  strip.show();
}

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  colorWipe(strip.Color(50, 0, 0), 10); // Red
  colorWipe(strip.Color(0, 50, 0), 10); // Green
  colorWipe(strip.Color(0, 0, 50), 10); // Blue
  allPixelsToBrightness(0);
  lastFadeTime = millis() - 1000;
  lastLightTime = lastFadeTime;
}

void lightRandomPixel() {
  unsigned long now = millis();
  if(now - lastFadeTime >= 51) {
    for(uint16_t i=0; i<1; i++) {
      pixelToBrightness(random(NUM_PIXELS), 10 + random(30));
    }
    lastLightTime = millis();
  }
}

void fadeAllPixels() {
  unsigned long now = millis();
  if(now - lastFadeTime >= 50) {
    for(uint16_t i=0; i<60; i++) {
      pixelToBrightness(i, max(0, pixels[i] - 1));
    }
    lastFadeTime = millis();
  }
}

uint16_t randomPixel;
void loop() {
  // Some example procedures showing how to display to the pixels:
  // rainbow(20);
  // rainbowCycle(100);
//  setPixelRed(random(strip.numPixels()));
//  delay(WAIT);
//  setPixelGreen(random(strip.numPixels()));
//  delay(WAIT);
  
  lightRandomPixel();
  fadeAllPixels();
  strip.show();
}


void allPixels(uint32_t color) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void switchPixelTo(uint16_t pixel, uint32_t color) {
  strip.setPixelColor(pixel, BLACK);
  strip.show();
  delay(200);
  strip.setPixelColor(pixel, color);
  strip.show();
  delay(200);
  strip.setPixelColor(pixel, BLACK);
  strip.show();
  delay(200);
  strip.setPixelColor(pixel, color);
  strip.show();
  delay(200);
  strip.setPixelColor(pixel, BLACK);
  strip.show();
  delay(200);
  strip.setPixelColor(pixel, color);
  strip.show();
  delay(200);
}

void setPixelGreen(uint16_t pixel) {
  switchPixelTo(pixel, GREEN);
}

void setPixelRed(uint16_t pixel) {
  switchPixelTo(pixel, RED);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

