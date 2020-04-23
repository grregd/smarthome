#include <PCF8574.h>

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
    int m_state;
};
