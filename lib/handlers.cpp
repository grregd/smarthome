
#include "handlers.h"

#include <ESP8266WiFi.h>
#include <hardwareSerial.h>

#include <cstdio>
#include <string>

namespace Handlers
{
template <size_t N>
char *mysnprintf(char (&buffer)[N], const char *format, ...)
{
  va_list vl;
  va_start(vl, format);
  vsnprintf(buffer, N, format, vl);
  va_end(vl);

  return buffer;
}

using LogicPositive = LevelLogic<true>;
using LogicNegative = LevelLogic<false>;

Handler::Handler(int portNum, int initialState, const std::string &headerMarker, const std::string &num, bool logic,
                 const std::string &versionInfo)
    : m_portNum(portNum), m_initialState(initialState), m_headerMarkerOn(headerMarker + "on"), m_headerMarkerOff(headerMarker + "off"), m_num(num), m_versionInfo(versionInfo)
{
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

void Handler::handleInput(const std::string &header)
{
  m_outputHandler = [this](WiFiClient &client) { stdOutputHandler(client); };

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
    m_outputHandler = [this](WiFiClient &client) { configOutputHandler(client); };
  }
}

void Handler::handleOutput(WiFiClient &client)
{
  m_outputHandler(client);
}

void Handler::stdOutputHandler(WiFiClient &client)
{
  char buffer[128];

  client.println(mysnprintf(buffer, "<p>GPIO %s - State %s</p>\n", m_num.c_str(), m_outputState.c_str()));
  client.println(mysnprintf(buffer, "<p><a href=\"/%s", m_num.c_str()));

  if (m_outputState == "off")
  {
    client.println("/on\"><button class=\"button\">ON</button></a></p>");
  }
  else
  {
    client.println("/off\"><button class=\"button button2\">OFF</button></a></p>");
  }
}
void Handler::configOutputHandler(WiFiClient &client)
{
  char buffer[512];

  client.println(mysnprintf(buffer, "<p>%s</p>", m_versionInfo.c_str()));
  client.println(mysnprintf(buffer, "<p>portNum: %d</p>\n", m_portNum));
  client.println(mysnprintf(buffer, "<p>initialState: %d</p>\n", m_initialState));
  client.println(mysnprintf(buffer, "<p>num: %s</p>\n", m_num.c_str()));
  client.println(mysnprintf(buffer, "<p>headerMarkerOn: %s</p>\n", m_headerMarkerOn.c_str()));
}

void initAll(std::vector<Handler> &handlers)
{
  for (auto &handler : handlers)
  {
    handler.init();
  }
}

void handleAllInput(std::vector<Handler> &handlers, const std::string &header)
{
  for (auto &handler : handlers)
  {
    handler.handleInput(header);
  }
}

void handleAllOuput(WiFiClient &client, std::vector<Handler> &handlers)
{
  for (auto &handler : handlers)
  {
    handler.handleOutput(client);
  }
}

} // namespace Handlers
