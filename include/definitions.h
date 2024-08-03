#pragma once

#define PIN_PA6 0
#define PIN_PA7 1
#define PIN_PA1 2
#define PIN_PA2 3
#define PIN_PA3 4

#define IR_TRANS_PIN PIN_PA7

#define SLEEP_TIME 5e6 // Seconds * e6
#define TIME_ELAPSED_TO_SLEEP_MS 5000
#define PULSE_DELAY_MS 100
#define PULSE_HIGH_DELAY_MS 5
#define ALARM_LENGTH_MS 1000
#define TIMING_ARR_LENGTH 6

#define WIFI_MAX_CONNECT_ATTEMPTS 10 

#define MQTT_MAX_CONNECT_ATTEMPTS 10


#define IR_SENSOR_RECV
#define IS_WEMOS


#ifdef IS_WEMOS
  #include <ESP8266WiFi.h>
  // #define LED_PIN SCL
  #define LED_PIN LED_BUILTIN
  #define IR_RECV_PIN SDA

  const char* mqtt_server = "192.168.2.3";
  const int mqtt_port = 1883;
#else
  #define LED_PIN PIN_PA2
  #define IR_RECV_PIN PIN_PA1
#endif