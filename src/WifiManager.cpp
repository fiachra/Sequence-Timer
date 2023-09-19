#include "WifiManager.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;
WebServer server(80);
const uint32_t connectTimeoutMs = 20000;

void handleRoot()
{
  Serial.println("handle");
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP32!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

           hr, min % 60, sec % 60);
  server.send(200, "text/html", temp);
}

WifiManager::WifiManager(char *wifiConfig)
{
  this->configureWifi(wifiConfig);
  server.on("/", handleRoot);
  server.begin();
}

void WifiManager::update()
{
  server.handleClient();
}

void WifiManager::configureWifi(char *wifiConfig) {
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  DynamicJsonDocument doc(1024);

  File file = SPIFFS.open(wifiConfig, "r");

  String json = "";
  while (file.available())
  {
    json += file.readStringUntil('\n');
  }
  file.close();

  Serial.println(json);

  DeserializationError error = deserializeJson(doc, json);

  if (error)
  {
    Serial.println("ERROR");
    Serial.println(error.c_str());
    return;
  }

  JsonArray arr = doc.as<JsonArray>();

  for (JsonObject value : arr)
  {
    const char* ssid = value["ssid"];
    const char* pass = value["password"];
    wifiMulti.addAP(ssid, pass);
    Serial.println(ssid);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin("34Ryebridge", "welcometothirtyfour");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());

  // if(wifiMulti.run() == WL_CONNECTED) {
  //   Serial.println("");
  //   Serial.println("WiFi connected");
  //   Serial.println("IP address: ");
  //   Serial.println(WiFi.localIP());
  // }
}
