/*********
  Based on
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <string>
#include <vector>

// Replace with your network credentials
const char* ssid     = "";
const char* password = "";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

class Handler
{
public:
  Handler(int portNum, int initialState, const std::string& headerMarker, const std::string & num)
  : m_portNum(portNum)
  , m_initialState(initialState)
  , m_headerMarkerOn(headerMarker+"on")
  , m_headerMarkerOff(headerMarker+"off")
  , m_num(num)
  {
  }

  void init()
  {
    pinMode(m_portNum, OUTPUT);
    setOutputState(m_initialState);
  }

  void setOutputState(int state)
  {
    if (state == LOW)
      m_outputState = "off";
    else
      m_outputState = "on";

    digitalWrite(m_portNum, state);
  }

  void handleInput()
  {
    if (header.indexOf(m_headerMarkerOn.c_str()) >= 0)
    {
    //              Serial.println("GPIO 5 on");

      setOutputState(HIGH);
    }
    else if (header.indexOf(m_headerMarkerOff.c_str()) >= 0)
    {
    //              Serial.println("GPIO 5 off");
      setOutputState(LOW);
    }
  }

  void handleOutput(WiFiClient & client)
  {
    client.print("<p>GPIO ");
    client.print(m_num.c_str());
    client.println(" - State ");
    client.print(m_outputState.c_str());
    client.println("</p>");
    client.print("<p><a href=\"/");
    client.print(m_num.c_str());
    if (m_outputState == "off")
    {
      client.println("/on\"><button class=\"button\">ON</button></a></p>");
    }
    else
    {
      client.println("/off\"><button class=\"button button2\">OFF</button></a></p>");
    }
  }

private:
  int m_portNum;
  int m_initialState;
  std::string m_headerMarkerOn;
  std::string m_headerMarkerOff;
  std::string m_outputState;
  std::string m_num;
};

void setupPort(int port)
{
  pinMode(port, OUTPUT);
  digitalWrite(port, LOW);
}


void initAll(std::vector<Handler>& handlers)
{
  for (auto & handler: handlers)
  {
    handler.init();
  }
}

void handleAllInput(std::vector<Handler>& handlers)
{
  for (auto & handler: handlers)
  {
    handler.handleInput();
  }
}

void handleAllOuput(WiFiClient & client, std::vector<Handler>& handlers)
{
  for (auto & handler: handlers)
  {
    handler.handleOutput(client);
  }
}

std::vector<Handler> handlers{
    Handler(2, LOW, "GET /4/", "4")
  , Handler(3, HIGH, "GET /5/", "5")
};



void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(__LINE__);
  delay(10);
  // Initialize the output variables as outputs
  Serial.println(__LINE__);
  delay(10);

  initAll(handlers);

  // Connect to Wi-Fi network with SSID and password
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}



void loop()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  delay(100);

  if (client) {                             // If a new client connects,
    Serial.println(__LINE__);
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
//        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            handleAllInput(handlers);

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");

            handleAllOuput(client, handlers);

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

