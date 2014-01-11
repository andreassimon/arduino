#include <SPI.h>
#include <Ethernet.h>

const int VERTICAL_TAB = 13;


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,178,230);

EthernetClient client;


int redLed = 4;
int greenLed = 3;
int blueLed = 2;

void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  delay(2000);
  // http://slashjenkins.slashhosting.de
  if(client.connect("slashjenkins.slashhosting.de" , 80)) {
    Serial.println("connected");
    client.println("GET /jenkins/job/mediamonitor3-quellendatenbank/api/json HTTP/1.0");
    client.println("Host: slashjenkins.slashhosting.de");
    client.println();
  } else {
    Serial.println("connection failed");
  }
}

void parseKey(EthernetClient *client);

void parseList(EthernetClient *client) {
  Serial.write('[');
  char c;

  while((*client).available()) {
    c = (*client).read();
    Serial.write(c);
    if(c == ']') {
      Serial.println();
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
  Serial.write('{');
  char c;

  while((*client).available()) {
    c = (*client).read();
    Serial.write(c);
    if(c == '}') {
      Serial.println();
      (*client).read(); // skip ','
      (*client).read(); // skip '"'
      parseKey(client);
      break;
    }
  }
}

void parseString(EthernetClient *client) {
  String value;
  char c;

  while((*client).available()) {
    c = (*client).read();
    if(c == '"') {
      (*client).read(); // skip ','
      (*client).read(); // skip '"'
      Serial.println(value);
      parseKey(client);
      break;
    }
    value += c;
  }
}

void parseLiteral(EthernetClient *client, char openingChar) {
  String value;
  value += openingChar;
  char c;

  while((*client).available()) {
    c = (*client).read();
    if(c == ',') {
      (*client).read(); // skip '"'
      Serial.println(value);
      parseKey(client);
      break;
    }
    value += c;
  }
}

void parseValue(EthernetClient *client) {
  char closingChar;
  char openingChar = (*client).read();
  if(openingChar == '[') {
    parseList(client);
  } else if(openingChar == '"') {
    parseString(client);
  } else if(openingChar == '{') {
    parseObject(client);
  } else {
    parseLiteral(client, openingChar);
  }
}

void parseKey(EthernetClient *client) {
  String key;
  char c;

  while((*client).available()) {
    c = (*client).read();
    if(c == '"') {
      (*client).read(); // skip ':'
      Serial.print(key);
      Serial.print(": ");
      parseValue(client);
      break;
    }
    key += c;
  }
}

void parseBody(EthernetClient *client) {
  char c;

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
  char c;

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
  skipHeader(&client);

  if (!client.connected()) {
    client.stop();
    for(;;)
      ;
  }
}

// vim:ft=cpp
