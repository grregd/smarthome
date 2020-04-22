#pragma once

#include <ESP8266WiFi.h>
#include <PCF8574.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class WiFiClient;

namespace Handlers {

template <bool logic>
struct LevelLogic;

template <>
struct LevelLogic<true> {
    static constexpr auto Off = LOW;
    static constexpr auto On = HIGH;
};

template <>
struct LevelLogic<false> {
    static constexpr auto Off = HIGH;
    static constexpr auto On = LOW;
};

class OutputDevice {
public:
    virtual void set(int state) = 0;
    virtual int get() = 0;
    virtual const char* name() const = 0;
};

class BoardOutputDevice : public OutputDevice {
public:
    BoardOutputDevice(int portNum);

    void set(int state) override;
    virtual int get() override;
    const char* name() const override;

private:
    int m_portNum;
};

class PcfOutputDevice : public OutputDevice {
public:
    PcfOutputDevice(PCF8574& pcfDevice, int portNum);

    void set(int state) override;
    virtual int get() override;
    const char* name() const override;

private:
    PCF8574& m_pcfDevice;
    int m_portNum;
};

class Handler {
public:
    Handler(std::unique_ptr<OutputDevice>&& outputDevice,
        int portNum,
        int initialState,
        const std::string& headerMarker,
        const std::string& num,
        bool logic,
        const std::string& versionInfo = std::string());

    void init();

    void setOutputState(int state);

    void handleInput(const std::string& header);

    void handleOutput(WiFiClient& client);

private:
    void stdOutputHandler(WiFiClient& client);
    void configOutputHandler(WiFiClient& client);

private:
    std::unique_ptr<OutputDevice> m_outputDevice;
    int m_portNum = 0;
    int m_initialState = 0;
    std::string m_headerMarkerOn;
    std::string m_headerMarkerOff;
    std::string m_outputState;
    std::string m_num;
    std::string m_versionInfo;
    std::function<void(WiFiClient&)> m_outputHandler;
};

void initAll(std::vector<Handler>& handlers);

void handleAllInput(std::vector<Handler>& handlers, const std::string& header);

void handleAllOuput(WiFiClient& client, std::vector<Handler>& handlers);

} // namespace Handlers
