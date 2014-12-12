#include <Ethernet.h>
#include <JenkinsJobParser.h>
#include <Adafruit_NeoPixel.h>
#include "lib/Animations/Constant.cpp"
#include "lib/Animations/KnightRider.cpp"


// #define DEBUG
#define PIN 6
#define NUM_PIXELS 40

class Job {
  const char* host;
  int port;
  const char* uri;
  uint16_t firstPixel;
  uint16_t lastPixel;
  Adafruit_NeoPixel* strip;
  uint32_t green,
           red,
           white,
           ledColor;
  Animations::Animation* animation;

  public:
  Job(const char* host, int port, const char* uri, uint16_t firstPixel, uint16_t lastPixel, Adafruit_NeoPixel* strip) {
    (*this).host = host;
    (*this).port = port;
    (*this).uri = uri;
    (*this).firstPixel = firstPixel;
    (*this).lastPixel = lastPixel;
    (*this).strip = strip;
    (*this).green = (*strip).Color(  0,  50,   0);
    (*this).red =   (*strip).Color(128,   0,   0);
    (*this).white = (*strip).Color( 20,  20,  20);
  }

  const char* getHost() { return host; }
  int getPort() { return port; }
  const char* getUri() { return uri; }
  uint16_t getFirstPixel() { return firstPixel; }
  uint16_t getLastPixel() { return lastPixel; }

  uint32_t ledColorFromJobState(String jobState) {
    if(String("blue") == jobState ||
        String("blue_anime") == jobState)
      return green;
    if(String("red") == jobState ||
        String("red_anime") == jobState)
      return red;
    return white;
  }

  Animations::Animation* animationFromJobState(String jobState) {
    if(String("blue") == jobState) {
      return new Animations::Constant(strip, firstPixel, lastPixel, green);
    }
    if(String("blue_anime") == jobState) {
      return new Animations::KnightRider(strip, firstPixel, lastPixel, (*strip).Color(0, 192, 0), (*strip).Color(0, 60, 0));
    }
    if(String("red") == jobState) {
      return new Animations::Constant(strip, firstPixel, lastPixel, red);
    }
    if(String("red_anime") == jobState) {
      return new Animations::KnightRider(strip, firstPixel, lastPixel, (*strip).Color(192, 0, 0), (*strip).Color(60, 0, 0));
    }
  }

  // Deprecated
  void setPixels(uint16_t firstPixel, uint16_t lastPixel, uint32_t color) {
    for(uint16_t pixel = firstPixel; pixel <= lastPixel; pixel++) {
      (*strip).setPixelColor(pixel, color);
    }
    (*strip).show();
  }

  // Deprecated
  void setBuildColor(String buildColor) {
    Serial.print(" => ");
    ledColor = ledColorFromJobState(buildColor);
    Serial.print(" setPixels( ");
    Serial.print(firstPixel);
    Serial.print(", ");
    Serial.print(lastPixel);
    Serial.print(", ");
    Serial.print(ledColor);
    Serial.println(" )");
    setPixels(firstPixel, lastPixel, ledColor);
    Serial.println();
  }

  void updateBuildColor(String buildColor) {
    (*this).animation = animationFromJobState(buildColor);
  }
};

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);


// Enter the IP address for your controller below.
// The IP address is dependent on your local network:
// IPAddress ip(192,168,178,230);

  // DEV / TEST
  const IPAddress ip(10,10,11,13);
  const char* jenkins = "10.10.11.16";
  const Job jobs[] = {
    Job( jenkins, 8080, "/job/blue/api/json",  0,  9, &strip ),
    Job( jenkins, 8080, "/job/blue-animated/api/json", 10, 19, &strip ),
    Job( jenkins, 8080, "/job/red/api/json", 20, 29, &strip ),
    Job( jenkins, 8080, "/job/red-animated/api/json", 30, 39, &strip )
  };

/*
  // Gruenspar
  const IPAddress ip(192,168,1,128);
  const char *jenkins = "192.168.1.140";
  const Job jobs[] = {
    { jenkins, 8080, "/job/earth-mage_ALL_Build/api/json",  0, 14, &strip },
    { jenkins, 8080, "/job/earth-mage_ALL_UnitTests/api/json", 15, 29, &strip }
  };
*/

const int numberOfJobs = sizeof(jobs) / sizeof(Job);

int currentJobIndex = 0;
Job currentJob = jobs[currentJobIndex];

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

#define WAIT  400

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

JenkinsJobParser parser;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // start the Ethernet connection:
//  if (Ethernet.begin(mac) == 0) {
//    Serial.println("Failed to configure Ethernet using DHCP");
//    Serial.print("Try now to connect with IP ");
//    Serial.println(ip);
    Ethernet.begin(mac, ip);
//  }
  Serial.print("Connected with IP ");
  Serial.println(Ethernet.localIP());

  parser = JenkinsJobParser();
  // colorWipe(strip.Color(128,   0,   0), 50); // red
  // colorWipe(strip.Color(  0,  50,   0), 50); // green
  // colorWipe(strip.Color(  0,   0,  30), 50); // blue
  // colorWipe(strip.Color( 20,  20,  20), 50); // white
  // colorWipe(strip.Color(  0,   0,   0), 50); // black
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

void loop() {
#ifdef DEBUG
  printEthernetState(__LINE__);
#endif

  if(!client.connected()) {
    client.stop();

    currentJob.setBuildColor(parser.getColor());

    currentJobIndex++;
    if(currentJobIndex >= numberOfJobs) {
      currentJobIndex = 0;
    }
    currentJob = jobs[currentJobIndex];

    GET(currentJob.getHost(), currentJob.getPort(), currentJob.getUri());
    parser.reset();
  }
  if(client.available()) {
    parser.processResponseChar(client.read());
  }

#ifdef DEBUG
  printEthernetState(__LINE__);
#endif
}

// vim:ft=cpp
