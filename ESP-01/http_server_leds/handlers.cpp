
#include "handlers.h"

#include <ESP8266WiFi.h>

#include <string>

namespace Handlers
{

Handler::Handler(int portNum, int initialState, const std::string& headerMarker, const std::string & num)
  : m_portNum(portNum)
  , m_initialState(initialState)
  , m_headerMarkerOn(headerMarker+"on")
  , m_headerMarkerOff(headerMarker+"off")
  , m_num(num)
  {
    m_stdHandler = [this](WiFiClient& client){ stdOutputHandler(client); };
    m_configHandler = [this](WiFiClient& client){ configOutputHandler(client); };
    m_outputHandler = m_stdHandler;
  }

void Handler::init()
{
  pinMode(m_portNum, OUTPUT);
  setOutputState(m_initialState);
}

void Handler::setOutputState(int state)
{
  if (state == LOW)
    m_outputState = "off";
  else
    m_outputState = "on";

  digitalWrite(m_portNum, state);
}

void Handler::handleInput(const std::string & header)
{
  m_outputHandler = m_stdHandler;
  if (header.find(m_headerMarkerOn.c_str()) != std::string::npos)
  {
    setOutputState(HIGH);
  }
  else if (header.find(m_headerMarkerOff.c_str()) != std::string::npos)
  {
    setOutputState(LOW);
  }
  else if (header.find("GET /config") != std::string::npos)
  {
    m_outputHandler = m_configHandler;
  }
}


void Handler::handleOutput(WiFiClient & client)
{
  m_outputHandler(client);
}

void Handler::stdOutputHandler(WiFiClient & client)
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
void Handler::configOutputHandler(WiFiClient & client)
{
  client.print("<p>portNum: ");
  client.print(m_portNum);
  client.print("</p><br>");
  client.print("initialState: ");
  client.print(m_initialState);
  client.print("</p><br>");
}

}
