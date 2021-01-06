#include <ESP8266WiFi.h>

const char* wifiStatusText(int wifiStatus)
{
    switch (wifiStatus) {
    case WL_CONNECTED:
        return "WL_CONNECTED";
        break;
    case WL_NO_SHIELD:
        return "WL_NO_SHIELD";
        break;
    case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS";
        break;
    case WL_NO_SSID_AVAIL:
        return "WL_NO_SSID_AVAIL";
        break;
    case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED";
        break;
    case WL_CONNECT_FAILED:
        return "WL_CONNECT_FAILED";
        break;
    case WL_CONNECTION_LOST:
        return "WL_CONNECTION_LOST";
        break;
    case WL_DISCONNECTED:
        return "WL_DISCONNECTED";
        break;
    }

    return "unrecognized";
}
