/*********
  Based on
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

#include <handlers.h>
#include <html_fragments.h>
#include <parsing_config.h>

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <string>
#include <vector>

using namespace Handlers;

std::vector<Handler> handlers;

// Replace with your network credentials
const char* ssid = "";
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

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println(__LINE__);
    delay(10);
    // Initialize the output variables as outputs
    Serial.println(__LINE__);
    delay(10);

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

    String ipStr { WiFi.localIP().toString() };

    parseConfig(
        fetchConfig(ipStr.substring(ipStr.lastIndexOf('.') + 1)),
        [](int port, int activeLevel,
            const String& headerMarker,
            const String& num, bool logic) {
            handlers.push_back(Handler(port, activeLevel,
                headerMarker.c_str(), num.c_str(), logic));
        });

    initAll(handlers);

    server.begin();
}

void loop()
{
    WiFiClient client = server.available(); // Listen for incoming clients

    delay(100);

    if (client) { // If a new client connects,
        Serial.println(__LINE__);
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = ""; // make a String to hold incoming data from the client
        currentTime = millis();
        previousTime = currentTime;
        while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
            currentTime = millis();
            if (client.available()) { // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                header += c;
                if (c == '\n') { // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println(http200);
                        client.println();

                        handleAllInput(handlers, header);

                        client.println(htmlHead);
                        client.println(htmlBodyBeg);
                        handleAllOuput(client, handlers);
                        client.println(htmlBodyEnd);

                        // The HTTP response ends with another blank line
                        client.println();
                        // Break out of the while loop
                        break;
                    } else { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                } else if (c != '\r') { // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
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
