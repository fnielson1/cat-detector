#include <Arduino.h>

#define PIN_PA6 0
#define PIN_PA7 1
#define PIN_PA1 2
#define PIN_PA2 3
#define PIN_PA3 4

#define IR_TRANS_PIN PIN_PA7

#define PULSE_DELAY_MS 100
#define PULSE_HIGH_DELAY_MS 5
#define TIMING_ARR_LENGTH 3
#define ALARM_LENGTH_MS 1000

#define IS_WEMOS
#define IR_SENSOR_RECV

#ifdef IS_WEMOS
  #define LED_PIN SCL
  #define IR_RECV_PIN SDA
#else
  #define LED_PIN PIN_PA2
  #define IR_RECV_PIN PIN_PA1
#endif



void checkIfIrSignalReceived();
void transmitIrSignal();
void checkIfCommandReceived();


const int PULSE_FUDGE_FACTOR = 50 + PULSE_HIGH_DELAY_MS;
unsigned long timeSinceLastIrSignal = 0;
bool lastSignalWasHigh = false;
int timingArr[TIMING_ARR_LENGTH];
int timingArrIndex = 0;


void setup() {
  #ifdef IS_WEMOS
    Serial.begin(9600);
  #endif

  pinMode(LED_PIN, OUTPUT);

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

  // Ignore a high signal if the last signal was high
  if (signalDetected && lastSignalWasHigh) {
    return;
  }
  if (signalDetected) 
  {
    unsigned long timeDiff = currentTime - timeSinceLastIrSignal;
    lastSignalWasHigh = true;

    // We shouldn't go into here unless the time from the last signal till now is at least the length of the pulse delay
    if (timeDiff >= PULSE_DELAY_MS)
    {
      Serial.println(timeDiff);
      Serial.println("");

      timingArr[timingArrIndex++] = currentTime;
      timeSinceLastIrSignal = currentTime;

      if (timingArrIndex == TIMING_ARR_LENGTH)
      {
        timingArrIndex = 0;
        checkIfCommandReceived();
      }
    }
  }
  else {
    lastSignalWasHigh = false;
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
    digitalWrite(LED_PIN, HIGH);
    delay(ALARM_LENGTH_MS);
  }
  digitalWrite(LED_PIN, LOW);
}