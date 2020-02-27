
// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

static const char* VERSION = 
  "Version: ESP-01+PCF8574 v. 0.1\n"
  "Build date: " __DATE__ "\n"
  "Build time: " __TIME__ "\n";

static const char* ssid     = "";
static const char* password = "";

// Set web server port number to 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  
  Serial.println(VERSION);

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
  delay(1000);
}
