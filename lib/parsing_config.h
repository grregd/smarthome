#include <Arduino.h>
#include <functional>

String fetchConfig(const String& id);

void parseConfig(const String& configText,
    std::function<void(const String&,
        int port,
        int activeLevel,
        const String& headerMarker,
        const String& num,
        bool logic)>
        callback);
