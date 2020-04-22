
static const char http200[] = "HTTP/1.1 200 OK \
    Content-type:text/html \
    Connection: close";

static const char htmlHead[] = "<!DOCTYPE html><html> \
    <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> \
    <link rel=\"icon\" href=\"data:,\"> \
    <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} \
    .button { background-color: #195B6A; border: none; color: white; padding: 16px 40px; \
    text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;} \
    .button2 {background-color: #77878A;}</style></head>";

static const char htmlBodyBeg[] = "<body><h1>ESP8266 Web Server</h1>";

static const char htmlBodyEnd[] = "</body></html>";

static const char htmlHandlersNumberBeg[] = "<p>Number of handlers: ";
static const char htmlHandlersNumberEnd[] = "</p>";
