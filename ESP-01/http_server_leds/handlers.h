
#pragma once


#include <string>
#include <functional>

class WiFiClient;


namespace Handlers 
{
class Handler
{
public:
  Handler(int portNum, int initialState, const std::string& headerMarker, const std::string & num);

  void init();

  void setOutputState(int state);

  void handleInput(const std::string & header);

  void handleOutput(WiFiClient & client);

private:
  void stdOutputHandler(WiFiClient & client);
  void configOutputHandler(WiFiClient & client);

private:
  int m_portNum;
  int m_initialState;
  std::string m_headerMarkerOn;
  std::string m_headerMarkerOff;
  std::string m_outputState;
  std::string m_num;
  std::function<void(WiFiClient &)> m_stdHandler;
  std::function<void(WiFiClient &)> m_configHandler;
  std::function<void(WiFiClient &)> m_outputHandler;
};

}
