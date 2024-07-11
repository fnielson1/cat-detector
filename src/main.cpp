#include <Arduino.h>

#define PIN_PA6 0
#define PIN_PA7 1
#define PIN_PA1 2
#define PIN_PA2 3
#define PIN_PA3 4

#define IR_TRANS_PIN PIN_PA7

#define PULSE_DELAY_MS 500
#define PULSE_HIGH_DELAY 5
#define TIMING_ARR_LENGTH 3
#define ALARM_LENGTH_MS 500

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


const int PULSE_FUDGE_FACTOR = 50 + PULSE_HIGH_DELAY;
unsigned long timeSinceLastIrSignal = 0;
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
  delay(PULSE_HIGH_DELAY);
  digitalWrite(IR_TRANS_PIN, LOW);
  delay(PULSE_DELAY_MS);
}

void checkIfIrSignalReceived() {
  int isSignalNotDetected = digitalRead(IR_RECV_PIN);


  if (!isSignalNotDetected) {
    unsigned long currentTime = millis();
    unsigned long minusFudgeFactorTime = currentTime - PULSE_FUDGE_FACTOR;
    unsigned long plusFudgeFactorTime = currentTime + PULSE_FUDGE_FACTOR;
    unsigned long timeDiff = currentTime - timeSinceLastIrSignal;
    unsigned long timeDiffFromFudgeFactor = PULSE_FUDGE_FACTOR - timeDiff;

    Serial.println(timeDiff);
    Serial.println("");

    if (timeDiff >= 0 && timeDiff <= PULSE_FUDGE_FACTOR)
    {
      timingArr[timingArrIndex++] = timeDiff;

      if (timingArrIndex == TIMING_ARR_LENGTH)
      {
        timingArrIndex = 0;
        checkIfCommandReceived();
      }
      delay(PULSE_DELAY_MS - timeDiffFromFudgeFactor);
    }
    else
    {
      timeSinceLastIrSignal = currentTime;
    }
  }
}

void checkIfCommandReceived() {
  bool isReceived = true;

  for (int i = 0; i < TIMING_ARR_LENGTH; i++) {
    int value = timingArr[0];
    if (value > PULSE_FUDGE_FACTOR) {
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