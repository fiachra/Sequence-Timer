#include <Arduino.h>
#include "TimerControl.h"
#include "WifiManager.h"

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#define LED_PIN 22  // The built-in LED for many ESP32 boards is on pin 2
const int relayPin = D5;

TimerControl* timerControl;
WifiManager *wifiMan;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("34Ryebridge", "XX");
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
   Serial.println(WiFi.localIP());
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  wifiMan = new WifiManager("/wifi_networks.json");

  bool hasWifi = WiFi.status() == WL_CONNECTED;
  timerControl = new TimerControl("/timing.json", hasWifi);
  Serial.println("setup Complete");
  
}


void loop() {
  wifiMan->update();
  timerControl->update();
  delay(250);


 
  // delay(1000);
  // digitalWrite(relayPin, HIGH);
  // delay(1000);
  digitalWrite(relayPin, LOW);
}


