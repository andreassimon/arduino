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

  unsigned long t = millis(); while((!(*client).available()) && ((millis() - t) < responseTimeout)) ;
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


const byte PARSER_IN_HEADER = 0;
const byte PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY = 1;
const byte PARSER_IN_BODY = 2;
const byte PARSER_IN_KEY = 3;
const byte PARSER_AFTER_KEY = 4;
const byte PARSER_BEFORE_VALUE = 5;
const byte PARSER_IN_DELIMITED_VALUE = 6;
const byte PARSER_IN_LITERAL_VALUE = 7;
const byte PARSER_VALUE_CLOSED = 10;
const byte PARSER_ERROR = 255;
byte parserState;

void processHeaderChar(const char c) {
  switch(c) {
    case VERTICAL_TAB:
      return;
    case '\n':
      if(parserState == PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY) {
        parserState = PARSER_IN_BODY;
        return;
      }
      parserState = PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY;
      return;
    default:
      parserState = PARSER_IN_HEADER;
  }
}

void processBodyChar(const char c) {
  switch(c) {
    case '"':
      parserState = PARSER_IN_KEY;
  }
}

void processKeyChar(const char c) {
  switch(c) {
    case '"':
      Serial.println();
      parserState = PARSER_AFTER_KEY;
      return;
  }
  Serial.print(c);
}

const int MAX_DELIMITERS = 20;
char* openingDelimiters = (char*)malloc(MAX_DELIMITERS);
int lastDelimiterIndex = -1;

void resetDelimiters() {
  lastDelimiterIndex = -1;
}

boolean noDelimitersOpen() {
  return lastDelimiterIndex < 0;
}

void pushDelimiter(const char c) {
  lastDelimiterIndex++;
  if(lastDelimiterIndex >= MAX_DELIMITERS) {
    Serial.print("ERROR: The maximum number of delimiters (");
    Serial.print(MAX_DELIMITERS);
    Serial.print(") is exceeded, you try to push '");
    Serial.print(c);
    Serial.println("'");
    parserState = PARSER_ERROR;
    return;
  }
  openingDelimiters[lastDelimiterIndex] = c;
}

void popDelimiter() {
  lastDelimiterIndex--;
}

char peekDelimiter() {
  return openingDelimiters[lastDelimiterIndex];
}

void processDelimitedValueChar(const char c) {
  char lastDelimiter;
  switch(c) {
    case '[':
    case '{':
      pushDelimiter(c);
      break;
    case '"':
      if('"' == peekDelimiter()) {
        popDelimiter();
      } else {
        pushDelimiter(c);
      }
      break;
    case ']':
      lastDelimiter = peekDelimiter();
      if('[' == lastDelimiter) {
        popDelimiter();
      } else {
        Serial.print("'");
        Serial.print(openingDelimiters);
        Serial.println("'");
        Serial.print("ERROR: Parser expected last opening delimiter to be '[', but was '");
        Serial.print(lastDelimiter);
        Serial.println("'");
        parserState = PARSER_ERROR;
      }
      break;
    case '}':
      lastDelimiter = peekDelimiter();
      if('{' == lastDelimiter) {
        popDelimiter();
      } else {
        Serial.print("ERROR: Parser expected last opening delimiter to be '{', but was '");
        Serial.print(lastDelimiter);
        Serial.println("'");
        parserState = PARSER_ERROR;
      }
      break;
  }
  if(noDelimitersOpen()) {
    parserState = PARSER_VALUE_CLOSED;
  }
}

char* expectedLiteralValue = (char*)malloc(6);
int nextExpectedLiteralValueIndex = 0;

void processResponseChar(const char c) {
  switch(parserState) {
    case PARSER_IN_HEADER:
    case PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY:
      processHeaderChar(c);
      return;
    case PARSER_IN_BODY:
      processBodyChar(c);
      return;
    case PARSER_IN_KEY:
      processKeyChar(c);
      return;
    case PARSER_AFTER_KEY:
      if(':' != c) {
        Serial.print("ERROR: Parser expected ':', but found '");
        Serial.print(c);
        Serial.println("'");
        parserState = PARSER_ERROR;
        return;
      }
      parserState = PARSER_BEFORE_VALUE;
      return;
    case PARSER_BEFORE_VALUE:
      switch(c) {
        case '{':
        case '[':
        case '}':
        case ']':
        case '"':
          parserState = PARSER_IN_DELIMITED_VALUE;
          resetDelimiters();
          processDelimitedValueChar(c);
          break;
        case 'n':
          expectedLiteralValue = (char*)"null";
          nextExpectedLiteralValueIndex = 1;
          parserState = PARSER_IN_LITERAL_VALUE;
          break;
        case 't':
          expectedLiteralValue = (char*)"true";
          nextExpectedLiteralValueIndex = 1;
          parserState = PARSER_IN_LITERAL_VALUE;
          break;
        default:
          Serial.print("ERROR: Parser can not detect JSON value starting with '");
          Serial.print(c);
          Serial.println("'");
          parserState = PARSER_ERROR;
      }
      return;
    case PARSER_IN_LITERAL_VALUE:
      if(expectedLiteralValue[nextExpectedLiteralValueIndex] == c) {
        nextExpectedLiteralValueIndex++;

        if(nextExpectedLiteralValueIndex >= strlen(expectedLiteralValue)) {
          parserState = PARSER_VALUE_CLOSED;
        }
      } else {
        Serial.print("ERROR: Parser expected '");
        Serial.print(expectedLiteralValue[nextExpectedLiteralValueIndex]);
        Serial.print("' as part of literal '");
        Serial.print(expectedLiteralValue);
        Serial.print("', but found '");
        Serial.print(c);
        Serial.println("'");
        parserState = PARSER_ERROR;
      }
      return;
    case PARSER_IN_DELIMITED_VALUE:
      processDelimitedValueChar(c);
      return;
    case PARSER_VALUE_CLOSED:
      if(',' != c) {
        Serial.print("ERROR: Parser expected ',', but found '");
        Serial.print(c);
        Serial.println("'");
        parserState = PARSER_ERROR;
      } else {
        parserState = PARSER_IN_BODY;
      }
      return;
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
