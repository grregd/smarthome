#include "output_device.h"

BoardOutputDevice::BoardOutputDevice(int portNum)
    : m_portNum(portNum)
{
    pinMode(m_portNum, OUTPUT);
}

void BoardOutputDevice::set(int state)
{
    digitalWrite(m_portNum, state);
}

int BoardOutputDevice::get()
{
    return digitalRead(m_portNum);
}

const char* BoardOutputDevice::name() const
{
    return "BoardDevice";
}

PcfOutputDevice::PcfOutputDevice(PCF8574& pcfDevice, int portNum)
    : m_pcfDevice(pcfDevice)
    , m_portNum(portNum)
    , m_state(LOW)
{
    pcfDevice.pinMode(m_portNum, OUTPUT);
}

void PcfOutputDevice::set(int state)
{
    m_state = state;
    m_pcfDevice.digitalWrite(m_portNum, state);
}

int PcfOutputDevice::get()
{
    return m_state;
}

const char* PcfOutputDevice::name() const
{
    return "PcfDevice";
}
