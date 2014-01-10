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
  public:
    String method;

    int parse(EthernetClient *client) {
      String token = String();

      while ((*client).connected()) {
        if(!(*client).available()) {
          break;
        }

        char c = (*client).read();

        if(c == ' ') {
          method = token;
          break;
        }
        token += c;
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        // if (c == '\n') {
        // Serial.println("\n\n\n<RED>");
        // Serial.println(red("r", &request));
        // Serial.println("\n\n\n</RED>");
        // Serial.println(green(request));
        // Serial.println(blue(request));
        // send a standard http response header
        /* break; */
        // }
        //   if (c == '\n') {
        //     // you're starting a new line
        //     currentLineIsBlank = true;
        //   }
        //   else if (c != '\r') {
        //     // you've gotten a character on the current line
        //     currentLineIsBlank = false;
        //   }
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
    Serial.println(request.method);
    // Serial.println(request.length());
    // Serial.println(request);
    // free(&request);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<body>");
    client.println("<form action=\"/\" method=\"POST\">");
    client.println("<input type=\"text\" name=\"text\">");
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
