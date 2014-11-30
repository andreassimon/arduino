#include <SPI.h>
#include <Ethernet.h>

#define DEBUG

const int VERTICAL_TAB = 13;


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// IPAddress ip(192,168,178,230);
IPAddress ip(10,10,11,13);

const char *jenkins = "10.10.11.16";
// const char *jenkins = "slashjenkins.slashhosting.de";
const char *job1 = "/jenkins/job/blue-job/api/json";
const char *job2 = "/jenkins/job/red-job/api/json";

EthernetClient client;


void GET(const char **host, const char **uri) {
  int port = 8080;
  int connStatus = client.connect(*host, port);
  if(connStatus >= 0) {
#ifdef DEBUG
    Serial.print("GET http://");
    Serial.print(*host);
    Serial.print(":");
    Serial.print(port);
    Serial.print(*uri);
    Serial.println();
#endif

    client.print("GET ");
    client.print(*uri);
    client.println(" HTTP/1.0");
    client.print("Host: ");
    client.println(*host);
    client.println();
  } else {
#ifdef DEBUG
    Serial.print("HTTP connection failed: ");
    Serial.println(connStatus);
#endif
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

  delay(2000);
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
  value = String();

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

void skipHeader(EthernetClient *client) {
  boolean currentRowIsEmpty = true;

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

void loop() {
  Serial.println();
  Serial.println();

  GET(&jenkins, &job1);
  skipHeader(&client);

  if (!client.connected()) {
    client.stop();
    Serial.print(" job1 -> ");
    Serial.println(value);
  }
  delay(2500);

  Serial.println();
  Serial.println();

  GET(&jenkins, &job2);
  skipHeader(&client);

  if (!client.connected()) {
    client.stop();
    Serial.print(" job2 -> ");
    Serial.println(value);
  }
  delay(2500);
}

// vim:ft=cpp
