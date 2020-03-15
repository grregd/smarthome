//#define PCF8574_LOW_MEMORY
#define PCF8574_DEBUG
#define SDA 0
#define SCL 2

//#include <Wire.h>
#include <PCF8574.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

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

void setup() {
//  Serial.begin(115200);
//  
//  Serial.println(VERSION);
//
//  Serial.print("SDA: ");
//  Serial.println(SDA);
//  Serial.print("SCL: ");
//  Serial.println(SCL);
//  
//
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
  

  // Connect to Wi-Fi network with SSID and password
//  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
//    Serial.print(".");
  }


//  // Print local IP address and start web server
//  Serial.println("");
//  Serial.println("WiFi connected.");
//  Serial.println("IP address: ");
//  Serial.println(WiFi.localIP());

  server.begin();

}


byte stateLights[4] = {1, 1, 1, 1};
PCF8574::DigitalInput currentInput{};
PCF8574::DigitalInput previousInput{};
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
