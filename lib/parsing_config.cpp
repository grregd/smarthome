#include "parsing_config.h"

#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <string>
#include <vector>

String fetchConfig(const String& id)
{

    //  "https://raw.githubusercontent.com/grregd/smarthome/master/README.md?token=AAG2525BSVQHFDAQ5WDI6JK6IXB6M";
    String url = F("/grregd/smarthome/master/configs/config_");
    url += id;
    static const char host[] = "raw.githubusercontent.com";
    const int httpsPort = 443;

    static const char fingerprint[] PROGMEM = "CC:AA:48:48:66:46:0E:91:53:2C:9C:7C:23:2A:B1:74:4D:29:9D:33";

    Serial.print(F("fetchConfig from "));
    Serial.println(String(host) + url);

    WiFiClientSecure client;

    Serial.println(__LINE__);

    if (!client.setFingerprint(fingerprint)) {
        Serial.println(F("setFingerprint failed"));
        return {};
    }

    Serial.println(__LINE__);

    if (!client.connect(host, httpsPort)) {
        Serial.println(F("connection failed"));
        return {};
    }

    Serial.println(__LINE__);
    Serial.flush();

    if (client.verify(fingerprint, host)) {
        Serial.println(F("certificate matches"));
    } else {
        Serial.println(F("certificate doesn't match"));
    }

    Serial.println(__LINE__);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");

    Serial.println(__LINE__);

    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            Serial.println(F("headers received"));
            Serial.println(line);
            break;
        }
    }

    Serial.println(__LINE__);

    String result;
    while (client.available()) {
        String line = client.readStringUntil('\n');
        result += line;
        Serial.println(line);
        //    if (line.startsWith("{\"state\":\"success\"")) {
        //      Serial.println("esp8266/Arduino CI successfull!");
        //    } else {
        //      Serial.println("esp8266/Arduino CI has failed");
        //    }
    }

    Serial.println(F("fetchConfig success"));

    return result;
}

void parseConfig(const String& configText,
    std::function<void(
        const String& deviceName,
        int port,
        int activeLevel,
        const String& headerMarker,
        const String& num,
        bool logic)>
        callback)
{
    StaticJsonDocument<1024> configDocument;

    auto error = deserializeJson(configDocument, configText);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());

        return;
    }

    auto config = configDocument[F("config")];
    Serial.println(config[F("ModuleName")].as<String>());
    auto outputs = config[F("outputs")].as<JsonArray>();
    for (const JsonObject& output : outputs) {
        Serial.println(F("."));
        callback(
            output[F("device")].as<String>(),
            output[F("port")].as<int>(),
            output[F("initial")].as<String>() == "LOW" ? LOW : HIGH,
            output[F("headerMarker")].as<String>(),
            output[F("num")].as<String>(),
            true);
    }
}
