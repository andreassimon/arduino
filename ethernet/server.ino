#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,178,230);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

String queryParams(String *request) {
  int firstSpace = (*request).indexOf(' ');
  String uri = (*request).substring(firstSpace+1, (*request).indexOf(' ', firstSpace+1));
  return uri.substring(uri.indexOf('?')+1);
}

String red(String _variableName, String *request) {
  String variableName = _variableName + "=";
  String queryParameters = queryParams(request);
  int indexOfRed = queryParameters.indexOf(variableName);
  int nextAmp = queryParameters.indexOf('&', indexOfRed+1);
  return queryParameters.substring(indexOfRed, nextAmp);
}

class Request {
  String token;

  char nextChar(EthernetClient *client) {
    char c = (*client).read();
    Serial.write(c);
    return c;
  }

  int parseURI(EthernetClient *client) {
    token = String();

    while ((*client).connected()) {
      if(!(*client).available()) {
        break;
      }

      char c = nextChar(client);
      if(c == ' ') {
        uri = token;
        readRemainingHeaders(client);
        break;
      }
      token += c;
    }

    return 0;
  }

  int readRemainingHeaders(EthernetClient *client) {
    boolean currentLineIsBlank = true;

    while ((*client).connected()) {
      if(!(*client).available()) {
        break;
      }

      char c = nextChar(client);
      if(c == '\n') {
        if(currentLineIsBlank) {
          readRemaining(client);
        }
        currentLineIsBlank = true;
      } else {
        currentLineIsBlank = false;
      }
    }

    return 0;
  }

  int readRemaining(EthernetClient *client) {
    while ((*client).connected()) {
      if(!(*client).available()) {
        break;
      }

      nextChar(client);
    }

    return 0;
  }

  public:
    String method;
    String uri;

    int parse(EthernetClient *client) {
      token = String();

      while ((*client).connected()) {
        if(!(*client).available()) {
          break;
        }

        char c = nextChar(client);
        if(c == ' ') {
          method = token;
          parseURI(client);
          break;
        }
        token += c;
      }
      return 0;
    }
};

String request;
int requestLength;
void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    request = String("");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    Request request = Request();
    int status = request.parse(&client);
    delay(10);
    Serial.println();
    Serial.println();
    Serial.print("METHOD >>> ");
    Serial.println(request.method);
    Serial.print("URI    >>> ");
    Serial.println(request.uri);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<body>");
    client.println("<form action=\"/\" method=\"POST\">");
    client.println("<input type=\"text\" name=\"foo\" placeholder=\"foo\">");
    client.println("<input type=\"text\" name=\"bar\" placeholder=\"bar\">");
    client.println("<input type=\"submit\" value=\"Senden\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");

    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

// vim:ft=cpp
