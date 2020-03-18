
#define xstr(s) str(s)
#define str(s) #s

//#define PCF8574_LOW_MEMORY
#define PCF8574_DEBUG
#define SDA 0
#define SCL 2

#include <html_fragments.h>
#include <parsing_config.h>
#include <handlers.h>


//#include <Wire.h>
#include <PCF8574.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>



std::string header;

using namespace Handlers;

std::vector<Handler> handlers;

static const auto ON = LOW;
static const auto OFF = HIGH;

static const char* VERSION = 
  "Version: ESP-01+PCF8574 v. 0.1\n"
  "Build date: " __DATE__ "\n"
  "Build time: " __TIME__ "\n";

static const char* ssid     = "";
static const char* password = "";

const auto BASE_ADDRESS_PCF  = 0b00111000u;
const auto IOMOD_ADDRESS_TWI = 0b00000111u | BASE_ADDRESS_PCF; 

PCF8574 pcf(IOMOD_ADDRESS_TWI, 0, 2);

// Set web server port number to 80
WiFiServer server(80);


void setupWifi()
{
  // Connect to Wi-Fi network with SSID and password
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  String ipStr{ WiFi.localIP().toString() };

  parseConfig(
    fetchConfig(ipStr.substring(ipStr.lastIndexOf('.') + 1)),
    [](int port, int activeLevel,
       const String & headerMarker,
       const String & num, bool logic)
      {
        handlers.push_back(Handler(port, activeLevel, 
            headerMarker.c_str(), num.c_str(), logic));
      }
  );

  initAll(handlers);

  server.begin();
}


void setupPcf()
{
//  Serial.println("pcf.begin()");
  pcf.begin();
//  Serial.println("pcf.pinMode()");
  pcf.pinMode(P0, INPUT);
  pcf.pinMode(P1, INPUT);
  pcf.pinMode(P2, INPUT);
  pcf.pinMode(P3, INPUT);
  
  pcf.pinMode(P4, OUTPUT);
  pcf.pinMode(P5, OUTPUT);
  pcf.pinMode(P6, OUTPUT);
  pcf.pinMode(P7, OUTPUT);
//
//  auto myblink = [&pcf](int portNum)
//  {
//    pcf.digitalWrite(portNum, ON);
//    delay(500);
//    pcf.digitalWrite(portNum, OFF);
//  };
//
//  myblink(P4);
//  myblink(P5);
//  myblink(P6);
//  myblink(P7);
  
//  Serial.println("Testing I2C");
//  for (auto i = 0; i < 10; ++i)
//  {
//    Serial.println(i);
//    auto di = pcf.digitalReadAll();
//    Serial.println(di, BIN);
//    delay(500);
//    pcf.digitalWrite(P4, LOW);
//    delay(500);
//    pcf.digitalWrite(P4, HIGH);
//  }
//  Serial.println("Done testing I2C");
}

void setup()
{
  setupWifi();
  setupPcf(); 
}


byte stateLights[4] = {1, 1, 1, 1};
PCF8574::DigitalInput currentInput{};
PCF8574::DigitalInput previousInput{};

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void handleHttpChannel()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  delay(100);

  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println(http200.c_str());
            client.println();

            handleAllInput(handlers, header);
              
            client.println(htmlHead.c_str());
            client.println(htmlBodyBeg.c_str());
            handleAllOuput(client, handlers);
            client.println(htmlBodyEnd.c_str());

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
  }
}
void loop()
{
  const auto currentInput = pcf.digitalReadAll();
  const auto &di = currentInput;

  if (previousInput.p0 == 0 && currentInput.p0 == 1)
    stateLights[0] ^= 1;
  if (previousInput.p1 == 0 && currentInput.p1 == 1)
    stateLights[1] ^= 1;
  if (previousInput.p2 == 0 && currentInput.p2 == 1)
    stateLights[2] ^= 1;
  if (previousInput.p3 == 0 && currentInput.p3 == 1)
    stateLights[3] ^= 1;
  
  pcf.digitalWrite(P4, stateLights[0] == 0 ? HIGH : LOW);
  pcf.digitalWrite(P5, stateLights[1] == 0 ? HIGH : LOW);
  pcf.digitalWrite(P6, stateLights[2] == 0 ? HIGH : LOW);
  pcf.digitalWrite(P7, stateLights[3] == 0 ? HIGH : LOW);

  previousInput = currentInput;
  
  delay(100);
}
