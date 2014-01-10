#include <SPI.h>
#include <Ethernet.h>

const int VERTICAL_TAB = 13;


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,178,230);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

class Request {
  String token;

  char nextChar(EthernetClient *client) {
    char c = (*client).read();
    // Serial.write(c);
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
      if(int(c) == VERTICAL_TAB) { continue; }
      if(c == '\n') {
        if(currentLineIsBlank) {
          parseBody(client);
          break;
        }
        currentLineIsBlank = true;
      } else {
        currentLineIsBlank = false;
      }
    }

    return 0;
  }

  int parseBody(EthernetClient *client) {
    return parseVariable(client);
  }

  int parseVariable(EthernetClient *client) {
    token = String();

    while ((*client).connected()) {
      if(!(*client).available()) {
        break;
      }

      char c = nextChar(client);
      if(c == '=') {
        if(token == "red") {
          parseRedValue(client);
        } else if(token == "green") {
          parseGreenValue(client);
        } else if(token == "blue") {
          parseBlueValue(client);
        }
        break;
      }
      token += c;
    }
    return 0;
  }

  int parseRedValue(EthernetClient *client) {
    token = String();

    while ((*client).connected()) {
      if(!(*client).available()) {
        red = token;
        break;
      }

      char c = nextChar(client);
      if(c == '&') {
        red = token;
        parseVariable(client);
        break;
      }
      token += c;
    }
    return 0;
  }

  int parseGreenValue(EthernetClient *client) {
    token = String();

    while ((*client).connected()) {
      if(!(*client).available()) {
        green = token;
        break;
      }

      char c = nextChar(client);
      if(c == '&') {
        green = token;
        parseVariable(client);
        break;
      }
      token += c;
    }
    return 0;
  }

  int parseBlueValue(EthernetClient *client) {
    token = String();

    while ((*client).connected()) {
      if(!(*client).available()) {
        blue = token;
        break;
      }

      char c = nextChar(client);
      if(c == '&') {
        blue = token;
        parseVariable(client);
        break;
      }
      token += c;
    }
    return 0;
  }

  public:
    String method;
    String uri;
    String red;
    String green;
    String blue;

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


Request request;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection and the server:
  request = Request();
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

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    int status = request.parse(&client);
    delay(10);
    Serial.println();
    Serial.println();
    Serial.print("METHOD >>> ");
    Serial.println(request.method);
    Serial.print("URI    >>> ");
    Serial.println(request.uri);
    Serial.print("RED    >>> ");
    Serial.println(request.red);
    Serial.print("GREEN  >>> ");
    Serial.println(request.green);
    Serial.print("BLUE   >>> ");
    Serial.println(request.blue);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<body>");
    client.println("<form action=\"/\" method=\"POST\">");

    client.println("<label>Red: </label>");
    client.println("<select name=\"red\">");
    client.println("  <option value=\"on\">ON</option> ");
    client.println("  <option value=\"off\" selected>OFF</option>");
    client.println("</select>");
    client.println("<br>");

    client.println("<label>Green: </label>");
    client.println("<select name=\"green\">");
    client.println("  <option value=\"on\">ON</option> ");
    client.println("  <option value=\"off\" selected>OFF</option>");
    client.println("</select>");
    client.println("<br>");

    client.println("<label>Blue: </label>");
    client.println("<select name=\"blue\">");
    client.println("  <option value=\"on\">ON</option> ");
    client.println("  <option value=\"off\" selected>OFF</option>");
    client.println("</select>");
    client.println("<br>");

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
