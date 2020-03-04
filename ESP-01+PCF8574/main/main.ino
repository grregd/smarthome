#define PCF8574_LOW_MEMORY
#define PCF8574_DEBUG
#define SDA 0
#define SCL 2

//#include <Wire.h>
#include <PCF8574.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

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

void setup() {
  Serial.begin(115200);
  
  Serial.println(VERSION);

  Serial.print("SDA: ");
  Serial.println(SDA);
  Serial.print("SCL: ");
  Serial.println(SCL);
  

  Serial.println("pcf.begin()");
  pcf.begin();
  Serial.println("pcf.pinMode()");
  pcf.pinMode(P0, INPUT);
  pcf.pinMode(P1, INPUT);
  pcf.pinMode(P2, INPUT);
  pcf.pinMode(P3, INPUT);
  
  pcf.pinMode(P4, OUTPUT);
  pcf.pinMode(P5, OUTPUT);
  pcf.pinMode(P6, OUTPUT);
  pcf.pinMode(P7, OUTPUT);
  
  Serial.println("Testing I2C");
  for (auto i = 0; i < 10; ++i)
  {
    Serial.println(i);
    byte di = pcf.digitalReadAll();
    Serial.println(di, BIN);
    delay(500);
    pcf.digitalWrite(P4, LOW);
    delay(500);
    pcf.digitalWrite(P4, HIGH);
  }
  Serial.println("Done testing I2C");
  

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
