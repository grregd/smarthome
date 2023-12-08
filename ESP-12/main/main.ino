
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#define xstr(s) str(s)
#define str(s) #s

//#define PCF8574_LOW_MEMORY
#define PCF8574_DEBUG
#define SDA 0
#define SCL 2

#include <handlers.h>
#include <html_fragments.h>
#include <mysnprintf.h>
#include <parsing_config.h>
#include <wifi_status_txt.h>

//#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PCF8574.h>
#include <WiFiClientSecure.h>

using namespace Handlers;

std::vector<Handler> handlers;
std::map<int, std::shared_ptr<OutputDevice>> outputDevices;

static const auto ON = LOW;
static const auto OFF = HIGH;

static const char VERSION[] PROGMEM = "Version: ESP-12 v. 0.1\n"
                                      "Build date: " __DATE__ "\n"
                                      "Build time: " __TIME__ "\n";

static const char* ssid = "";
static const char* password = "";

const auto BASE_ADDRESS_PCF = 0b00111000u;
const auto IOMOD_ADDRESS_TWI = 0b00000111u | BASE_ADDRESS_PCF;

PCF8574 pcf(IOMOD_ADDRESS_TWI, 0, 2);

// Set web server port number to 80
WiFiServer server(80);

void setupWifi()
{
    Serial.println(F("setupWifi start"));

    Serial.print("ESP Board MAC Address:  ");
    Serial.println(WiFi.macAddress());
    // Connect to Wi-Fi network with SSID and password
    switch (WiFi.begin(ssid, password)) {
    case WL_CONNECTED:
        Serial.println(F("WL_CONNECTED"));
        break;
    case WL_IDLE_STATUS:
        Serial.println(F("WL_IDLE_STATUS"));
        break;
    }

    int wifiStatus = 0;
    do {
        wifiStatus = WiFi.status();
        Serial.println(wifiStatusText(wifiStatus));
        if (wifiStatus != WL_CONNECTED) {
            delay(500);
        }
    } while (wifiStatus != WL_CONNECTED);

    String ipStr { WiFi.localIP().toString() };

    std::string versionInfo = VERSION;
    versionInfo += ", ip: ";
    versionInfo += WiFi.localIP().toString().c_str();

    while (!parseConfig(
        fetchConfig(ipStr.substring(ipStr.lastIndexOf('.') + 1)),
        [versionInfo](const String& deviceName, int port, int activeLevel, const String& headerMarker,
            const String& num, bool logic) {
            if (deviceName == "pcf") {
                outputDevices[port] = std::make_shared<PcfOutputDevice>(pcf, port);
            } else {
                outputDevices[port] = std::make_shared<BoardOutputDevice>(port);
            }

            Serial.print(F("adding handler: "));
            Serial.println(outputDevices[port]->name());

            handlers.push_back(Handler(outputDevices[port],
                port, activeLevel, headerMarker.c_str(),
                num.c_str(), logic, versionInfo));
        })) {
        delay(500);
    }

    Serial.println(F("initAll"));
    initAll(handlers);

    server.begin();

    Serial.println(F("setupWifi finished"));
}

void setupPcf()
{
    Serial.println(F("setupPcf start"));

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

    Serial.println(F("setupPcf finished"));
}

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void handleHttpChannel()
{
    std::string header;

    WiFiClient client = server.available(); // Listen for incoming clients

    if (client) { // If a new client connects,
        String currentLine = ""; // make a String to hold incoming data from the client
        currentTime = millis();
        previousTime = currentTime;
        while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
            currentTime = millis();
            if (client.available()) { // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                header += c;
                if (c == '\n') { // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a
                    // row. that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200
                        // OK) and a content-type so the client knows what's coming, then a
                        // blank line:
                        client.println(http200);
                        client.println();

                        handleAllInput(handlers, header);

                        client.println(htmlHead);
                        client.println(htmlBodyBeg);

                        client.println(htmlHandlersNumberBeg);
                        client.println(handlers.size());
                        client.println(htmlHandlersNumberEnd);

                        handleAllOuput(client, handlers);
                        client.println(htmlBodyEnd);

                        // The HTTP response ends with another blank line
                        client.println();
                        // Break out of the while loop
                        break;
                    } else { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                } else if (c != '\r') { // if you got anything else but a carriage
                    // return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        // Close the connection
        client.stop();
    }
}

PCF8574::DigitalInput currentInput {};
PCF8574::DigitalInput previousInput {};

void pcfPinToggle(PCF8574& pcf, int pin)
{
    outputDevices[pin]->set(outputDevices[pin]->get() == LOW ? HIGH : LOW);
}

void handlePcfChannel()
{
    const auto currentInput = pcf.digitalReadAll();
    const auto& di = currentInput;

    if (previousInput.p0 == 0 && currentInput.p0 == 1)
        pcfPinToggle(pcf, P4);
    if (previousInput.p1 == 0 && currentInput.p1 == 1)
        pcfPinToggle(pcf, P5);
    if (previousInput.p2 == 0 && currentInput.p2 == 1)
        pcfPinToggle(pcf, P6);
    if (previousInput.p3 == 0 && currentInput.p3 == 1)
        pcfPinToggle(pcf, P7);

    previousInput = currentInput;
}

void test1()
{
    std::vector<int> pins { P5, P4, 13, 12, P2, 10, 14, 16 };

    for (auto pin : pins) {
        pinMode(pin, OUTPUT);
    }

    while (true) {
        for (auto pin : pins) {
            digitalWrite(pin, 1);
            delay(500);
            digitalWrite(pin, 0);
            delay(500);
        }
    }
}

#define DEBOUNCE_TIME 250
volatile uint32_t DebounceTimer = 0;

void keyPressed()
{
    if (millis() - DEBOUNCE_TIME >= DebounceTimer) {
        DebounceTimer = millis();
        Serial.printf("Button has been pressed\n");
    }
}

ICACHE_RAM_ATTR void keyPressed_k1()
{
    Serial.println(__PRETTY_FUNCTION__);
    keyPressed();
}

ICACHE_RAM_ATTR void keyPressed_k2()
{
    Serial.println(__PRETTY_FUNCTION__);
    keyPressed();
}

ICACHE_RAM_ATTR void keyPressed_k3()
{
    Serial.println(__PRETTY_FUNCTION__);
    keyPressed();
}

ICACHE_RAM_ATTR void keyPressed_k4()
{
    Serial.println(__PRETTY_FUNCTION__);
    keyPressed();
}

void test2()
{
    std::vector<std::tuple<int, void (*)()>> keys_handlers {
        std::make_tuple(P2, keyPressed_k1),
        std::make_tuple(10, keyPressed_k2),
        std::make_tuple(14, keyPressed_k3),
        std::make_tuple(16, keyPressed_k4)
    };

    char buffer[128];
    for (const auto& handler : keys_handlers) {

        Serial.println(mysnprintf(buffer,
            "Setting up handler for key %d", std::get<0>(handler)));

        pinMode(std::get<0>(handler), INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(std::get<0>(handler)),
            std::get<1>(handler), FALLING);
    }

    while (true)
        delay(500);
}

bool in_range(int value, int reference, int margin)
{
    return ((reference - margin) < value) && (value < (reference + margin));
}

void test3()
{
    std::vector<std::tuple<int, const char*, int>> keysInfo {
        std::make_tuple(1024 * 8 / 8, "key1", P5),
        std::make_tuple(1024 * 7 / 8 + 41, "key2", P4),
        std::make_tuple(1024 * 6 / 8 + 61, "key3", P2),
        std::make_tuple(1024 * 5 / 8 + 30, "key4", 10),
        std::make_tuple(1024 * 4 / 8 - 41, "key5", 13),
        std::make_tuple(1024 * 3 / 8 - 20, "key6", 12),
        std::make_tuple(1024 * 2 / 8 - 10, "key7", 14),
        std::make_tuple(1024 * 1 / 8 + 10, "key8", 16)
    };

    for (const auto keyInfo : keysInfo) {
        pinMode(std::get<2>(keyInfo), OUTPUT);
    }

    while (true) {
        int voltage = analogRead(A0);

        if (voltage == 0) {
            for (const auto keyInfo : keysInfo) {
                digitalWrite(std::get<2>(keyInfo), 0);
            }
        } else {
            Serial.printf("%d\n", voltage);
            for (const auto keyInfo : keysInfo) {
                if (in_range(voltage, std::get<0>(keyInfo), 1024 * 1 / 8 / 3)) {
                    Serial.printf("%s key pressed\n", std::get<1>(keyInfo));
                    digitalWrite(std::get<2>(keyInfo), 1);
                }
            }
        }

        delay(100);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println(VERSION);
    Serial.println(F("setup start"));

    test3();
    return;

    setupWifi();
    setupPcf();

    Serial.println(F("setup finished"));
    Serial.flush();
    Serial.end();
}

void loop()
{
    // handleHttpChannel();
    // handlePcfChannel();
    delay(100);
}
