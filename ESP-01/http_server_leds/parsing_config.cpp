#include "parsing_config.h"

#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <string>
#include <vector>

String fetchConfig(const String & id)
{

//  "https://raw.githubusercontent.com/grregd/smarthome/master/README.md?token=AAG2525BSVQHFDAQ5WDI6JK6IXB6M";
  String url = "/grregd/smarthome/master/configs/config_" + id;
  static const char host[] = "raw.githubusercontent.com";
  const int httpsPort = 443;

  static const char fingerprint[] = "CC:AA:48:48:66:46:0E:91:53:2C:9C:7C:23:2A:B1:74:4D:29:9D:33";

  Serial.print("fetchConfig from ");
  Serial.println(String(host) + url);

  WiFiClientSecure client;

  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return {};
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }


  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      Serial.println(line);
      break;
    }
  }

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

  Serial.println("fetchConfig success");

  return result;
}


void parseConfig( const String & configText,
                  std::function<void(int port,
                                     int activeLevel,
                                     const String& headerMarker,
                                     const String& num,
                                     bool logic)> callback)
{
  StaticJsonDocument<1024> configDocument;

  auto error = deserializeJson(configDocument, configText);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  auto config = configDocument["config"];
  Serial.println(config["ModuleName"].as<String>());
  auto outputs = config["outputs"].as<JsonArray>();
  for (const JsonObject & output : outputs)
  {
    Serial.println(".");
    callback(
      output["port"].as<int>(),
      output["initial"].as<String>() == "LOW" ? LOW : HIGH,
      output["headerMarker"].as<String>(),
      output["num"].as<String>(),
      true );
  }
}

