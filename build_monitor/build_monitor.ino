#include <SPI.h>
#include <Ethernet.h>

#include <Adafruit_NeoPixel.h>


// #define DEBUG

// Enter the IP address for your controller below.
// The IP address is dependent on your local network:
// IPAddress ip(192,168,178,230);

// DEV / TEST
const char *jenkins = "10.10.11.16";
const int port = 8080;
const char *job1 = "/job/earth-mage_ALL_Build/api/json";
const char *job2 = "/job/earth-mage_ALL_UnitTests/api/json";
const IPAddress ip(10,10,11,13);

// Gruenspar
/*
  const char *jenkins = "192.168.1.140";
  const int port = 8080;
  const char *job1 = "/job/earth-mage_ALL_Build/api/json";
  const char *job2 = "/job/earth-mage_ALL_UnitTests/api/json";
*/

// Slashjenkins
/*
  const char *jenkins = "slashjenkins.slashhosting.de";
*/

struct Job {
  const char** host;
  const char** uri;
  uint16_t firstPixel;
  uint16_t lastPixel;
};

const Job j1 = { &jenkins, &job1,  0, 14 };
const Job j2 = { &jenkins, &job2, 15, 29 };

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const unsigned long responseTimeout = 15L * 1000L;
const int VERTICAL_TAB = 13;

#define PIN 6
#define NUM_PIXELS 30
#define BLACK strip.Color(  0,   0,   0)
#define WHITE strip.Color( 20,  20,  20)
#define GREEN strip.Color(  0,  50,   0)
#define BLUE  strip.Color(  0,   0,  30)
#define RED   strip.Color(128,   0,   0)
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

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  colorWipe(RED  , 50);
  colorWipe(GREEN, 50);
  colorWipe(BLUE , 50);
  colorWipe(WHITE, 50);
  colorWipe(BLACK, 50);
}

char c;
void parseKey(EthernetClient *client);

void parseList(EthernetClient *client) {
  while((*client).connected() || (*client).available()) {
    if((*client).available()) {
      c = (*client).read();
      if(c == ']') {
        if((*client).available()) {
          c = (*client).read(); // skip ','
        }
        if((*client).available()) {
          c = (*client).read(); // skip '"'
        }
        parseKey(client);
        break;
      }
    } else {
      // Serial.print(__LINE__);
      // Serial.println(" :: Waiting for input from Ethernet");
      // delay(250);
    }
  }
}

void parseObject(EthernetClient *client) {
  while((*client).connected() || (*client).available()) {
    if((*client).available()) {
      c = (*client).read();
      if(c == '}') {
        if((*client).available()) {
          c = (*client).read(); // skip ','
        }
        if((*client).available()) {
          c = (*client).read(); // skip '"'
        }
        parseKey(client);
        break;
      }
    } else {
      Serial.print(__LINE__);
      Serial.println(" :: Waiting for input from Ethernet");
      delay(250);
    }
  }
}

String value;
void parseColor(EthernetClient *client) {
  while((*client).connected() || (*client).available()) {
    if((*client).available()) {
      c = (*client).read();
      if(c == '"') {
        if((*client).available()) {
          c = (*client).read(); // skip ','
        }
        if((*client).available()) {
          c = (*client).read(); // skip '"'
        }
        parseKey(client);
        break;
      }
      value += c;
    } else {
      Serial.print(__LINE__);
      Serial.println(" :: Waiting for input from Ethernet");
      delay(250);
    }
  }
}

void parseString(EthernetClient *client) {

  while((*client).connected() || (*client).available()) {
    if((*client).available()) {
      c = (*client).read();
      if(c == '"') {
        if((*client).available()) {
          c = (*client).read(); // skip ','
        }
        if((*client).available()) {
          c = (*client).read(); // skip '"'
        }
        parseKey(client);
        break;
      }
    } else {
      Serial.print(__LINE__);
      Serial.println(" :: Waiting for input from Ethernet");
      delay(250);
    }
  }
}

void parseLiteral(EthernetClient *client, char openingChar) {
  while((*client).connected() || (*client).available()) {
    if((*client).available()) {
      c = (*client).read();
      if(c == ',') {
        if((*client).available()) {
          c = (*client).read(); // skip '"'
        }
        parseKey(client);
        break;
      }
    } else {
      Serial.print(__LINE__);
      Serial.println(" :: Waiting for input from Ethernet");
      delay(250);
    }
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

  while((*client).connected() || (*client).available()) {
    if((*client).available()) {
      c = (*client).read();
      if(c == '"') {
        if((*client).available()) {
          c = (*client).read(); // skip ':'
        }
        if(key == "color") {
          c = (*client).read(); // skip '"'
          parseColor(client);
        } else {
          parseValue(client);
        }
        break;
      }
      key += c;
    } else {
      Serial.print(__LINE__);
      Serial.println(" :: Waiting for input from Ethernet");
      delay(250);
    }
  }
}

void parseBody(EthernetClient *client) {

  while((*client).connected() || (*client).available()) {
    if((*client).available()) {
      c = (*client).read();
      if(c == '"') {
        parseKey(client);
        break;
      }
    } else {
      Serial.print(__LINE__);
      Serial.println(" :: Waiting for input from Ethernet");
      delay(250);
    }
  }
}

boolean currentRowIsEmpty;

void parseResponse(EthernetClient *client) {
  value = String();
  currentRowIsEmpty = true;

  unsigned long t = millis(); while((!(*client).available()) && ((millis() - t) < responseTimeout));
  while((*client).connected() || (*client).available()) {
    if((*client).available()) {
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
    } else {
      Serial.print(__LINE__);
      Serial.println(" :: Waiting for input from Ethernet");
      delay(250);
    }
  }
}

void setPixels(uint16_t firstPixel, uint16_t lastPixel, uint32_t color) {
  for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
    strip.setPixelColor(pixel, color);
  }
  strip.show();
}

uint32_t ledColorFromJobState(String jobState) {
  if(String("blue") == value ||
     String("blue_anime") == value)
    return GREEN;
  if(String("red") == value ||
     String("red_anime") == value)
    return RED;
  return WHITE;
}

#ifdef DEBUG
void printEthernetState(const int line) {
  Serial.print(line);
  Serial.print(": client.available() => ");
  Serial.println(client.available());
  Serial.print(line);
  Serial.print(": client.connected() => ");
  Serial.println(client.connected());
}
#endif


const int PARSER_IN_HEADER = 0;
const int PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY = 1;
const int PARSER_IN_BODY = 2;
byte parserState;
void processResponseChar(char c) {
  if(parserState == PARSER_IN_BODY) {
    Serial.print(c);
    return;
  }

  if(c == VERTICAL_TAB)
    return;

  if(c == '\n') {
    if(currentRowIsEmpty) {
      parserState = PARSER_IN_BODY;
      // parseBody(client);
      return;
    }
    currentRowIsEmpty = true;
    parserState = PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY;
  } else {
    parserState = PARSER_IN_HEADER;
    currentRowIsEmpty = false;
  }
}

void loop() {
  uint32_t ledColor;

  // Serial.println();

//  GET(j1.host, j1.uri);
//  parseResponse(&client);

#ifdef DEBUG
  printEthernetState(__LINE__);
#endif

//  client.stop();
//  Serial.print(" => ");
//  Serial.println(value);

//  ledColor = ledColorFromJobState(value);
//  setPixels(j1.firstPixel, j1.lastPixel, ledColor);

// Repeat
  if(!client.connected()) {
    GET(j2.host, j2.uri);
    parserState = PARSER_IN_HEADER;
  }
  if(client.available()) {
    c = client.read();
    processResponseChar(c);
  }
  // parseResponse(&client);

#ifdef DEBUG
  printEthernetState(__LINE__);
#endif

  if(!client.connected()) {
    client.stop();
    Serial.println();
    Serial.print(" => ");
    Serial.println(value);
  }

  ledColor = ledColorFromJobState(value);
  setPixels(j2.firstPixel, j2.lastPixel, ledColor);
}

// vim:ft=c
