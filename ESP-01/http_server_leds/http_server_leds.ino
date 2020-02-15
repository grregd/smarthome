/*********
  Based on
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/


#include "handlers.h"

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <string>
#include <vector>
#include <ArduinoJson.h>


using namespace Handlers;


// Replace with your network credentials
const char* ssid     = "";
const char* password = "";

// Set web server port number to 80
WiFiServer server(80);



// Variable to store the HTTP request
std::string header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setupPort(int port)
{
  pinMode(port, OUTPUT);
  digitalWrite(port, LOW);
}



std::vector<Handler> handlers{
//    Handler(2, LOW, "GET /4/", "4", true)
//  , Handler(3, HIGH, "GET /5/", "5", true)
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

  String ipStr{ WiFi.localIP().toString() };

  parseConfig(
    fetchConfig(
      ipStr.substring(ipStr.lastIndexOf('.') + 1)),
      [](int port, int activeLevel,
        const String& headerMarker,
        const String& num, bool logic)
        {
          Serial.println(headerMarker);
          handlers.push_back(Handler{
            port, activeLevel, headerMarker.c_str(), num.c_str(), logic});
       });


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

            handleAllInput(handlers, header);

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

String fetchConfig(const String & id)
{

//  "https://raw.githubusercontent.com/grregd/smarthome/master/README.md?token=AAG2525BSVQHFDAQ5WDI6JK6IXB6M";
  String url = "/grregd/smarthome/master/configs/config_" + id;
  static const char host[] = "raw.githubusercontent.com";
  const int httpsPort = 443;

  static const char fingerprint[] = "CC:AA:48:48:66:46:0E:91:53:2C:9C:7C:23:2A:B1:74:4D:29:9D:33";

  Serial.print("fetchConfig from ");
  Serial.println(String(host)+url);

  WiFiClientSecure client;

  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return {};
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }


  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "User-Agent: BuildFailureDetectorESP8266\r\n" +
             "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      Serial.println(line);
      break;
    }
  }

  String result;
  while (client.available()) {
    String line = client.readStringUntil('\n');
    result += line;
    Serial.println(line);
//    if (line.startsWith("{\"state\":\"success\"")) {
//      Serial.println("esp8266/Arduino CI successfull!");
//    } else {
//      Serial.println("esp8266/Arduino CI has failed");
//    }
  }

  Serial.println("fetchConfig success");

  return result;
}

void parseConfig( const String & configText,
  std::function<void(int port,
                     int activeLevel,
                     const String& headerMarker,
                     const String& num,
                     bool logic)> callback)
{
  StaticJsonDocument<1024> configDocument;

  auto error = deserializeJson(configDocument, configText);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  auto config = configDocument["config"];
  Serial.println(config["ModuleName"].as<String>());
  auto outputs = config["outputs"].as<JsonArray>();
  for (const JsonObject & output: outputs)
  {
    Serial.println(".");
    callback(
      output["port"].as<int>(),
      output["initial"].as<String>() == "LOW" ? LOW : HIGH,
      output["headerMarker"].as<String>(),
      output["num"].as<String>(),
      true );
  }
}
