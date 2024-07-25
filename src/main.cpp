#include <Arduino.h>

#define PIN_PA6 0
#define PIN_PA7 1
#define PIN_PA1 2
#define PIN_PA2 3
#define PIN_PA3 4

#define IR_TRANS_PIN PIN_PA7

#define SLEEP_TIME 10e6
#define TIME_ELAPSED_TO_SLEEP_MS 5000
#define PULSE_DELAY_MS 100
#define PULSE_HIGH_DELAY_MS 5
#define ALARM_LENGTH_MS 1000
#define TIMING_ARR_LENGTH 6
#define NUMBER_OF_WIFI_CONNECT_ATTEMPTS 10 

#define IS_WEMOS
#define IR_SENSOR_RECV

#ifdef IS_WEMOS
  #include <ESP8266WiFi.h>
  #define LED_PIN SCL
  #define IR_RECV_PIN SDA
#else
  #define LED_PIN PIN_PA2
  #define IR_RECV_PIN PIN_PA1
#endif



void checkIfIrSignalReceived();
void transmitIrSignal();
void checkIfCommandReceived();
void connectToWifiAndTransmitSignal();
void pushToTimingArray(unsigned long);


const char ssid[] = "Monkey";
const char pass[] = "nielsonfamilyhome";
const int WAIT_FOR_PULSE_DELAY = PULSE_DELAY_MS / 2;
const int PULSE_FUDGE_FACTOR = 50 + PULSE_HIGH_DELAY_MS;
unsigned long timeSinceLastIrSignal = 0;
bool lastSignalWasHigh = false;
int timingArr[TIMING_ARR_LENGTH];
int timingArrIndex = 0;

IPAddress local_IP(192, 168, 2, 35);
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);

void setup() {
  pinMode(LED_PIN, OUTPUT);

  #ifdef IS_WEMOS
    // Don't connect GPIO 16 to RESET pin until after upload
    Serial.begin(9600);

    // Wait for serial to initialize.
    while (!Serial) { }

    // Set your Static IP address
     WiFi.mode(WIFI_STA);
    if (!WiFi.config(local_IP, gateway, subnet)) {
      Serial.println("STA Failed to configure");
    }

    // Put wifi to sleep to save power
    WiFi.forceSleepBegin();

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
  unsigned long timeDiff = currentTime - timeSinceLastIrSignal;


  // Ignore a high signal if the last signal was high
  if (signalDetected && !lastSignalWasHigh && timeDiff >= WAIT_FOR_PULSE_DELAY) 
  {
    // We shouldn't go into here unless the time from the last signal till now is at least the length of the pulse delay
    Serial.println("HIGH");
    Serial.println(timeDiff);
    Serial.println("");

    lastSignalWasHigh = true;
    timeSinceLastIrSignal = currentTime;

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

void connectToWifiAndTransmitSignal() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting");
    WiFi.begin(ssid, pass);
  }

  for (int i = NUMBER_OF_WIFI_CONNECT_ATTEMPTS; i >= 0; i--) {
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
    delay(ALARM_LENGTH_MS);
    Serial.println("Alarm done");

    timeSinceLastIrSignal = millis(); // Don't make us think we need to sleep as we were merely transmitting the alarm
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
    connectToWifiAndTransmitSignal();
  }
  digitalWrite(LED_PIN, LOW);
}