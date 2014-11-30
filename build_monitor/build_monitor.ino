#include <SPI.h>
#include <Ethernet.h>

#include <Adafruit_NeoPixel.h>

#define DEBUG

// Enter the IP address for your controller below.
// The IP address is dependent on your local network:
// IPAddress ip(192,168,178,230);
IPAddress ip(10,10,11,13);

const char *jenkins = "10.10.11.16";
// const char *jenkins = "slashjenkins.slashhosting.de";
const int port = 8080;
const char *job1 = "/jenkins/job/blue-job/api/json";
const char *job2 = "/jenkins/job/red-job/api/json";

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const unsigned long responseTimeout = 15L * 1000L;
const int VERTICAL_TAB = 13;

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
EthernetClient client;

void GET(const char **host, const char **uri) {
  int connStatus = client.connect(*host, port);
  if(connStatus >= 0) {
    Serial.print("GET http://");
    Serial.print(*host);
    Serial.print(":");
    Serial.print(port);
    Serial.print(*uri);
    Serial.println();

    client.print("GET ");
    client.print(*uri);
    client.println(" HTTP/1.0");
    client.print("Host: ");
    client.println(*host);
    client.println("User-Agent: Arduino Uno Build Monitor");
    client.println("Connection: close");
    client.println();
  } else {
#ifdef DEBUG
    Serial.print("HTTP connection failed: ");
    Serial.println(connStatus);
#endif
  }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  colorWipe(RED  , 50);
  colorWipe(GREEN, 50);
  colorWipe(BLUE , 50);
  colorWipe(BLACK, 50);
}

char c;
void parseKey(EthernetClient *client);

void parseList(EthernetClient *client) {
  /* Serial.write('['); */

  while((*client).available()) {
    c = (*client).read();
    /* Serial.write(c); */
    if(c == ']') {
      /* Serial.println(); */
      if((*client).available()) {
        (*client).read(); // skip ','
      }
      if((*client).available()) {
        (*client).read(); // skip '"'
      }
      parseKey(client);
      break;
    }
  }
}

void parseObject(EthernetClient *client) {
  /* Serial.write('{'); */

  while((*client).available()) {
    c = (*client).read();
    /* Serial.write(c); */
    if(c == '}') {
      /* Serial.println(); */
      if((*client).available()) {
        (*client).read(); // skip ','
      }
      if((*client).available()) {
        (*client).read(); // skip '"'
      }
      parseKey(client);
      break;
    }
  }
}

String value;
void parseColor(EthernetClient *client) {
  while((*client).available()) {
    c = (*client).read();
    if(c == '"') {
      if((*client).available()) {
        (*client).read(); // skip ','
      }
      if((*client).available()) {
        (*client).read(); // skip '"'
      }
      parseKey(client);
      break;
    }
    value += c;
  }
}

void parseString(EthernetClient *client) {

  while((*client).available()) {
    c = (*client).read();
    if(c == '"') {
      if((*client).available()) {
        (*client).read(); // skip ','
      }
      if((*client).available()) {
        (*client).read(); // skip '"'
      }
      /* Serial.println(); */
      parseKey(client);
      break;
    }
    /* Serial.write(c); */
  }
}

void parseLiteral(EthernetClient *client, char openingChar) {
  /* Serial.write(openingChar); */

  while((*client).available()) {
    c = (*client).read();
    if(c == ',') {
      if((*client).available()) {
        (*client).read(); // skip '"'
      }
      /* Serial.println(); */
      parseKey(client);
      break;
    }
    /* Serial.write(c); */
  }
}

void parseValue(EthernetClient *client) {
  c = (*client).read();
  if(c == '[') {
    parseList(client);
  } else if(c == '"') {
    parseString(client);
  } else if(c == '{') {
    parseObject(client);
  } else {
    parseLiteral(client, c);
  }
}

void parseKey(EthernetClient *client) {
  String key;

  while((*client).available()) {
    c = (*client).read();
    if(c == '"') {
      if((*client).available()) {
        (*client).read(); // skip ':'
      }
      /* Serial.print(key); */
      /* Serial.print(": "); */
      if(key == "color") {
        (*client).read(); // skip '"'
        parseColor(client);
      } else {
        parseValue(client);
      }
      break;
    }
    key += c;
  }
}

void parseBody(EthernetClient *client) {

  while((*client).available()) {
    c = (*client).read();
    if(c == '"') {
      parseKey(client);
      break;
    }
  }
}

void parseResponse(EthernetClient *client) {
  value = String();
  boolean currentRowIsEmpty = true;

  unsigned long t = millis(); while((!(*client).available()) && ((millis() - t) < responseTimeout));
  while((*client).available()) {
    c = (*client).read();
    if(c == VERTICAL_TAB) {
      continue;
    }
    if(c == '\n') {
      if(currentRowIsEmpty) {
        parseBody(client);
        break;
      }
      currentRowIsEmpty = true;
    } else {
      currentRowIsEmpty = false;
    }
  }
}

void setPixel(uint16_t pixel, uint32_t color) {
  strip.setPixelColor(pixel, color);
  strip.show();
}

void loop() {

#ifdef DEBUG
  Serial.println();
  Serial.println();
#endif

  GET(&jenkins, &job1);
  parseResponse(&client);

#ifdef DEBUG
  Serial.print(__LINE__);
  Serial.print(": client.available() => ");
  Serial.println(client.available());
  Serial.print(__LINE__);
  Serial.print(": client.connected() => ");
  Serial.println(client.connected());
#endif

  client.stop();
  Serial.print(" job1 -> ");
  Serial.println(value);

  if(String("blue") == value) {
    setPixel(1, GREEN);
    setPixel(2, GREEN);
    setPixel(3, GREEN);
  } else if(String("red") == value) {
    setPixel(1, RED);
    setPixel(2, RED);
    setPixel(3, RED);
  } else {
    setPixel(1, BLACK);
    setPixel(2, BLACK);
    setPixel(3, BLACK);
  }
  delay(2500);

  Serial.println();
  Serial.println();

  GET(&jenkins, &job2);
  parseResponse(&client);

#ifdef DEBUG
  Serial.print(__LINE__);
  Serial.print(": client.available() => ");
  Serial.println(client.available());
  Serial.print(__LINE__);
  Serial.print(": client.connected() => ");
  Serial.println(client.connected());
#endif

  client.stop();
  Serial.print(" job2 -> ");
  Serial.println(value);

  setPixel(4, RED);
  setPixel(5, RED);
  setPixel(6, RED);
}

// vim:ft=c
