#include <Arduino.h>
#include "definitions.h"
#include "../lib/mqtt.cpp"


unsigned long timeSinceLastIrSignal = 0;

#ifdef IS_WEMOS
  const char ssid[] = "Monkey";
  const char pass[] = "nielsonfamilyhome";
  IPAddress localIP(192, 168, 2, 35);
  IPAddress gateway(192, 168, 2, 1);
  IPAddress subnet(255, 255, 255, 0);
#endif


#ifdef IS_WEMOS 
  unsigned long getTimeSinceLastIrSignal() {
    return timeSinceLastIrSignal;
  }

  void setTimeSinceLastIrSignal(unsigned long time) {
    timeSinceLastIrSignal = time;
  }

  void connectToWifiAndTransmitSignal() {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Connecting to wifi");
      WiFi.begin(ssid, pass);
    }

    for (int i = WIFI_MAX_CONNECT_ATTEMPTS; i >= 0; i--) {
      if (WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_PIN, LOW);
        delay(250);
        Serial.print(".");
        digitalWrite(LED_PIN, HIGH);
        delay(250);
      }
      else {
        break;
      }
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Connected, IP address: ");
      Serial.println(WiFi.localIP());

      digitalWrite(LED_PIN, HIGH);
      transmitMqtt("CAT DETECTED!");
      delay(ALARM_LENGTH_MS);
      Serial.println("Alarm done");

      setTimeSinceLastIrSignal(millis()); // Don't make us think we need to sleep as we were merely transmitting the alarm
    }
  }

  void setupWifi() {
    WiFi.mode(WIFI_STA);
    if (!WiFi.config(localIP, gateway, subnet)) {
      Serial.println("STA Failed to configure");
    }

    // Put wifi to sleep to save power
    WiFi.forceSleepBegin();
  }
#endif