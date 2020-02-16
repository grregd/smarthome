
#pragma once


#include <string>
#include <vector>
#include <functional>
#include <ESP8266WiFi.h>

class WiFiClient;


namespace Handlers 
{

template< bool logic >
struct LevelLogic;

template<>
struct LevelLogic< true >
{
  static constexpr auto Off = LOW;
  static constexpr auto On = HIGH;
};

template<>
struct LevelLogic< false >
{
  static constexpr auto Off = HIGH;
  static constexpr auto On = LOW;
};


class Handler
{
public:
  Handler(int portNum, int initialState, const std::string& headerMarker, const std::string & num, bool logic);

  void init();

  void setOutputState(int state);

  void handleInput(const std::string & header);

  void handleOutput(WiFiClient & client);

private:
  void stdOutputHandler(WiFiClient & client);
  void configOutputHandler(WiFiClient & client);

private:
  int m_portNum = 0;
  int m_initialState = 0;
  std::string m_headerMarkerOn;
  std::string m_headerMarkerOff;
  std::string m_outputState;
  std::string m_num;
  std::function<void(WiFiClient &)> m_outputHandler;
};

void initAll(std::vector<Handler>& handlers);

void handleAllInput(std::vector<Handler>& handlers, const std::string & header);

void handleAllOuput(WiFiClient & client, std::vector<Handler>& handlers);


}
