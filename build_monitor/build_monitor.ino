#include <SPI.h>
#include <Ethernet.h>

#include <Adafruit_NeoPixel.h>


// #define DEBUG

struct Job {
  const char* host;
  int port;
  const char* uri;
  uint16_t firstPixel;
  uint16_t lastPixel;
};

// Enter the IP address for your controller below.
// The IP address is dependent on your local network:
// IPAddress ip(192,168,178,230);

/*
  // DEV / TEST
  const IPAddress ip(10,10,11,13);
  const char* jenkins = "10.10.11.16";
  const Job jobs[] = {
    { jenkins, 8080, "/job/earth-mage_ALL_Build/api/json",  0, 14 },
    { jenkins, 8080, "/job/earth-mage_ALL_UnitTests/api/json", 15, 29 }
  };
*/

  // Gruenspar
  const IPAddress ip(192,168,1,128);
  const char *jenkins = "192.168.1.140";
  const Job jobs[] = {
    { jenkins, 8080, "/job/earth-mage_ALL_Build/api/json",  0, 14 },
    { jenkins, 8080, "/job/earth-mage_ALL_UnitTests/api/json", 15, 29 }
  };

const int numberOfJobs = sizeof(jobs) / sizeof(Job);

int currentJobIndex = 0;
Job currentJob = jobs[currentJobIndex];

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

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

void GET(const char *host, const int port, const char *uri) {
  int connStatus = client.connect(host, port);
  if(connStatus >= 0) {
    Serial.print("GET http://");
    Serial.print(host);
    Serial.print(":");
    Serial.print(port);
    Serial.print(uri);
    Serial.println();

    client.print("GET ");
    client.print(uri);
    client.println(" HTTP/1.0");
    client.print("Host: ");
    client.println(host);
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

const byte PARSER_IN_HEADER = 0;
const byte PARSER_IN_HEADER__CURRENT_ROW_IS_EMPTY = 1;
const byte PARSER_IN_BODY = 2;
const byte PARSER_IN_KEY = 3;
const byte PARSER_AFTER_KEY = 4;
const byte PARSER_AFTER_COLOR_KEY = 5;
const byte PARSER_BEFORE_VALUE = 6;
const byte PARSER_BEFORE_COLOR_VALUE = 7;
const byte PARSER_IN_DELIMITED_VALUE = 8;
const byte PARSER_IN_COLOR_VALUE = 9;
const byte PARSER_IN_LITERAL_VALUE = 10;
const byte PARSER_IN_NUMERIC_VALUE = 11;
const byte PARSER_VALUE_CLOSED = 12;
const byte PARSER_FINISHED = 13;
const byte PARSER_ERROR = 255;

const unsigned int MAX_KEY_LEN = 5;
const signed int MAX_DELIMITERS = 20;

class JenkinsJobParser {

  byte parserState;
  char currentKey[MAX_KEY_LEN];
  unsigned int currentKeyIndex;
  char* openingDelimiters;
  int lastDelimiterIndex;
  char* expectedLiteralValue;
  unsigned int nextExpectedLiteralValueIndex;
  String color;

  public:
  JenkinsJobParser() {
    currentKeyIndex = 0;

    openingDelimiters = (char*)malloc(MAX_DELIMITERS);
    lastDelimiterIndex = -1;

    expectedLiteralValue = (char*)malloc(6);
    nextExpectedLiteralValueIndex = 0;

    color = String("");
  }

  String getColor() {
    return color;
  }

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

  void processKeyChar(const unsigned char c) {
    switch(c) {
      case '"':
        currentKey[currentKeyIndex] = '\0';
        if(strncmp("color", currentKey, strlen("color")) == 0) {
          parserState = PARSER_AFTER_COLOR_KEY;
        } else {
          parserState = PARSER_AFTER_KEY;
        }
        return;
    }
    if(currentKeyIndex < MAX_KEY_LEN) {
      currentKey[currentKeyIndex] = c;
      currentKeyIndex++;
    }
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

  void processColorChar(const char c) {
    if('"' == c) {
      parserState = PARSER_VALUE_CLOSED;
      return;
    }
    color += c;
  }

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
      case PARSER_AFTER_COLOR_KEY:
        if(':' != c) {
          Serial.print("ERROR: Parser expected ':', but found '");
          Serial.print(c);
          Serial.println("'");
          parserState = PARSER_ERROR;
          return;
        }
        parserState = PARSER_BEFORE_COLOR_VALUE;
        return;
      case PARSER_BEFORE_COLOR_VALUE:
        if('"' == c) {
          parserState = PARSER_IN_COLOR_VALUE;
          color = String("");
        } else {
          Serial.print("ERROR: Parser expected '\"', but found '");
          Serial.print(c);
          Serial.println("'");
          parserState = PARSER_ERROR;
        }
        return;
      case PARSER_IN_COLOR_VALUE:
        processColorChar(c);
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
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            parserState = PARSER_IN_NUMERIC_VALUE;
            break;
          case 'f':
            expectedLiteralValue = (char*)"false";
            nextExpectedLiteralValueIndex = 1;
            parserState = PARSER_IN_LITERAL_VALUE;
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
      case PARSER_IN_NUMERIC_VALUE:
        if('0' <= c && c <= '9') {
          return;
        } else if(',' == c) {
          parserState = PARSER_IN_BODY;
        }
        return;
      case PARSER_VALUE_CLOSED:
        switch(c) {
          case ',':
            parserState = PARSER_IN_BODY;
            break;
          case '}':
            parserState = PARSER_FINISHED;
            break;
          default:
            Serial.print("ERROR: Parser expected ',' or '}', but found '");
            Serial.print(c);
            Serial.println("'");
            parserState = PARSER_ERROR;
        }
        return;
      case PARSER_FINISHED:
        Serial.print("ERROR: Parser expected no more characters, but found '");
        Serial.print(c);
        Serial.println("'");
        parserState = PARSER_ERROR;
    }
  }

  void reset() {
    parserState = PARSER_IN_HEADER;
  }

  void resetDelimiters() {
    lastDelimiterIndex = -1;
  }

  boolean noDelimitersOpen() {
    return lastDelimiterIndex < 0;
  }

  void popDelimiter() {
    lastDelimiterIndex--;
  }

  char peekDelimiter() {
    return openingDelimiters[lastDelimiterIndex];
  }

};

JenkinsJobParser parser;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Serial.print("Try now to connect with IP ");
    Serial.println(ip);
    Ethernet.begin(mac, ip);
  }

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  parser = JenkinsJobParser();
  colorWipe(RED  , 50);
  colorWipe(GREEN, 50);
  colorWipe(BLUE , 50);
  colorWipe(WHITE, 50);
  colorWipe(BLACK, 50);
}

char c;

void setPixels(uint16_t firstPixel, uint16_t lastPixel, uint32_t color) {
  for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
    strip.setPixelColor(pixel, color);
  }
  strip.show();
}

uint32_t ledColorFromJobState(String jobState) {
  if(String("blue") == jobState ||
     String("blue_anime") == jobState)
    return GREEN;
  if(String("red") == jobState ||
     String("red_anime") == jobState)
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

uint32_t ledColor;
void loop() {
#ifdef DEBUG
  printEthernetState(__LINE__);
#endif

  if(!client.connected()) {
    client.stop();
    Serial.print(" => ");
    Serial.println(parser.getColor());
    ledColor = ledColorFromJobState(parser.getColor());
    Serial.print(" setPixels( ");
    Serial.print(currentJob.firstPixel);
    Serial.print(", ");
    Serial.print(currentJob.lastPixel);
    Serial.print(", ");
    Serial.print(ledColor);
    Serial.println(" )");
    setPixels(currentJob.firstPixel, currentJob.lastPixel, ledColor);
    Serial.println();

    currentJobIndex++;
    if(currentJobIndex >= numberOfJobs) {
      currentJobIndex = 0;
    }
    currentJob = jobs[currentJobIndex];

    GET(currentJob.host, currentJob.port, currentJob.uri);
    parser.reset();
  }
  if(client.available()) {
    c = client.read();
    parser.processResponseChar(c);
  }

#ifdef DEBUG
  printEthernetState(__LINE__);
#endif
}

// vim:ft=cpp
