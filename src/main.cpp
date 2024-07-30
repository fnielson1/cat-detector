#include <Arduino.h>
#include "definitions.h"
#include "../lib/wifi_utils.cpp"


void checkIfIrSignalReceived();
void transmitIrSignal();
void checkIfCommandReceived();
void pushToTimingArray(unsigned long);


const int WAIT_FOR_PULSE_DELAY = PULSE_DELAY_MS / 2;
const int PULSE_FUDGE_FACTOR = 50 + PULSE_HIGH_DELAY_MS;
bool lastSignalWasHigh = false;
int timingArr[TIMING_ARR_LENGTH];
int timingArrIndex = 0;


void setup() {
  pinMode(LED_PIN, OUTPUT);

  #ifdef IS_WEMOS
    // Don't connect GPIO 16 to RESET pin until after upload
    Serial.begin(9600);

    // Wait for serial to initialize.
    while (!Serial) { }

    setupWifi();
    setupMqtt();

    Serial.println("");
    Serial.println("Started up");
  #endif

  #ifdef IR_SENSOR_RECV
    pinMode(IR_RECV_PIN, INPUT);
  #else
    pinMode(IR_TRANS_PIN, OUTPUT);
  #endif
}


void loop() {
  #ifdef IR_SENSOR_RECV
    checkIfIrSignalReceived();
  #else
    transmitIrSignal();
  #endif
}


void transmitIrSignal() {
  digitalWrite(IR_TRANS_PIN, HIGH);
  delay(PULSE_HIGH_DELAY_MS);
  digitalWrite(IR_TRANS_PIN, LOW);
  delay(PULSE_DELAY_MS);
}

void checkIfIrSignalReceived() {
  int signalDetected = !digitalRead(IR_RECV_PIN);
  unsigned long currentTime = millis();
  unsigned long timeDiff = currentTime - getTimeSinceLastIrSignal();


  // Ignore a high signal if the last signal was high
  if (signalDetected && !lastSignalWasHigh && timeDiff >= WAIT_FOR_PULSE_DELAY) 
  {
    // We shouldn't go into here unless the time from the last signal till now is at least the length of the pulse delay
    Serial.println("HIGH");
    Serial.println(timeDiff);
    Serial.println("");

    lastSignalWasHigh = true;
    setTimeSinceLastIrSignal(currentTime);

    pushToTimingArray(currentTime);
  }
  else if (lastSignalWasHigh) {
    Serial.println("LOW");
    Serial.println(timeDiff);
    Serial.println("");

    lastSignalWasHigh = false;

    pushToTimingArray(currentTime);
  }
  else {
    if (timeDiff > TIME_ELAPSED_TO_SLEEP_MS) {
      Serial.println("Going to sleep");
      ESP.deepSleep(SLEEP_TIME, RF_NO_CAL);
    }
  }
}


void pushToTimingArray(unsigned long currentTime) {
  timingArr[timingArrIndex++] = currentTime;

  if (timingArrIndex == TIMING_ARR_LENGTH)
  {
    timingArrIndex = 0;
    checkIfCommandReceived();
  }
}

void checkIfCommandReceived() {
  bool isReceived = true;

  for (int i = 0; i < TIMING_ARR_LENGTH - 1; i++) {
    int value = timingArr[i];
    int nextValue = timingArr[i + 1];

    if (value - nextValue > PULSE_FUDGE_FACTOR) {
      isReceived = false;
      break;
    }
  }


  if (isReceived) {
    #ifdef IS_WEMOS
      connectToWifiAndTransmitSignal();
    #elif
      digitalWrite(LED_PIN, HIGH);
      delay(ALARM_LENGTH_MS);
    #endif
  }
  digitalWrite(LED_PIN, LOW);
}