#include <Ethernet.h>
#include <JenkinsJobParser.h>
#include <Adafruit_NeoPixel.h>
#include "lib/Animations/Constant.cpp"
#include "lib/Animations/Blink.cpp"
#include "lib/Animations/KnightRider.cpp"
#include "lib/Animations/PulsatingGreen.cpp"
#include "lib/Animations/PulsatingRed.cpp"


// #define DEBUG
#define PIN 6
#define NUM_PIXELS 58

class Job {
  const char* host;
  int port;
  const char* uri;
  uint16_t firstPixel;
  uint16_t lastPixel;
  Adafruit_NeoPixel* strip;
  Animations::Animation *animation,
                        *green,
                        *greenAnimated,
                        *red,
                        *redAnimated,
                        *unknown;

  public:
  Job(const char* host, int port, const char* uri, uint16_t firstPixel, uint16_t lastPixel, Adafruit_NeoPixel* strip) {
    (*this).host = host;
    (*this).port = port;
    (*this).uri = uri;
    (*this).firstPixel = firstPixel;
    (*this).lastPixel = lastPixel;
    (*this).strip = strip;
    (*this).green = new Animations::Constant(strip, firstPixel, lastPixel, 0x003200);
    // (*this).greenAnimated = new Animations::KnightRider(strip, firstPixel, lastPixel, 0x00c000, 0x003c00);
    (*this).greenAnimated = new Animations::PulsatingGreen(strip, firstPixel, lastPixel, 60, 200);
    (*this).red = new Animations::Constant(strip, firstPixel, lastPixel, 0x800000);
    // (*this).redAnimated = new Animations::KnightRider(strip, firstPixel, lastPixel, 0xc00000, 0x3c0000);
    (*this).redAnimated = new Animations::PulsatingRed(strip, firstPixel, lastPixel, 60, 200);
    (*this).unknown = new Animations::Constant(strip, firstPixel, lastPixel, 0x202020);
    (*this).animation = unknown;
  }

  const char* getHost() { return host; }
  int getPort() { return port; }
  const char* getUri() { return uri; }
  uint16_t getFirstPixel() { return firstPixel; }
  uint16_t getLastPixel() { return lastPixel; }

  Animations::Animation* animationFromJobState(String jobState) {
    if(String("blue") == jobState) {
      return green;
    }
    if(String("blue_anime") == jobState) {
      return greenAnimated;
    }
    if(String("red") == jobState) {
      return red;
    }
    if(String("red_anime") == jobState) {
      return redAnimated;
    }
    return unknown;
  }

  void setBuildColor(String buildColor) {
    (*this).animation = animationFromJobState(buildColor);
  }

  void updateAnimation() {
    (*animation).update();
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
  Job* jobs[] = {
    new Job( jenkins, 8080, "/job/blue/api/json",  0,  9, &strip ),
    new Job( jenkins, 8080, "/job/blue-animated/api/json", 10, 19, &strip ),
    new Job( jenkins, 8080, "/job/red/api/json", 20, 29, &strip ),
    new Job( jenkins, 8080, "/job/red-animated/api/json", 30, 39, &strip ),
    new Job( jenkins, 8080, "/job/earth-mage_ALL_UnitTests/api/json", 40, 49, &strip ),
    new Job( jenkins, 8080, "/job/red/api/json", 50, 57, &strip )
  };

/*
  // Gruenspar
  const IPAddress ip(192,168,1,128);
  const char *jenkins = "192.168.1.140";
  Job* jobs[] = {
    new Job( jenkins, 8080, "/job/earth-mage_ALL_Build/api/json",  0, 14, &strip ),
    new Job( jenkins, 8080, "/job/earth-mage_ALL_UnitTests/api/json", 15, 29, &strip )
  };
*/

const int numberOfJobs = sizeof(jobs) / sizeof(Job*);

int currentJobIndex = numberOfJobs-1;
Job* currentJob = jobs[currentJobIndex];

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

#define WAIT  400

EthernetClient client;

void GET(const char *host, const int port, const char *uri) {
  int connStatus = client.connect(host, port);
  if(connStatus >= 0) {
#ifdef DEBUG
    Serial.print("GET http://");
    Serial.print(host);
    Serial.print(":");
    Serial.print(port);
    Serial.print(uri);
    Serial.println();
#endif

    client.print("GET ");
    client.print(uri);
    client.print(" HTTP/1.0\nHost: ");
    client.print(host);
    client.print("\nUser-Agent: Arduino Build Monitor\nConnection: close\n\n");
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

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Serial.print("Try now to connect with IP ");
    Serial.println(ip);
    Ethernet.begin(mac, ip);
  }
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());

  parser = JenkinsJobParser();
  colorWipe(strip.Color(128,   0,   0), 50); // red
  colorWipe(strip.Color(  0,  50,   0), 50); // green
  colorWipe(strip.Color(  0,   0,  30), 50); // blue
  // colorWipe(strip.Color( 20,  20,  20), 50); // white
  colorWipe(strip.Color(  0,   0,   0), 50); // black
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

    (*currentJob).setBuildColor(parser.getColor());

    currentJobIndex++;
    if(currentJobIndex >= numberOfJobs) {
      currentJobIndex = 0;
    }
    currentJob = jobs[currentJobIndex];

    GET((*currentJob).getHost(), (*currentJob).getPort(), (*currentJob).getUri());
    parser.reset();
  }
  if(client.available()) {
    parser.processResponseChar(client.read());
  }
  for(int i = 0; i < numberOfJobs; i++) {
    (*jobs[i]).updateAnimation();
  }

#ifdef DEBUG
  printEthernetState(__LINE__);
#endif
}

// vim:ft=cpp
