#include <PubSubClient.h>
#include "definitions.h"
#include "mqtt.h"

/*
  Simple wemos D1 mini  MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to the provided access point using dhcp, using ssid and pswd

  It connects to an MQTT server ( using mqtt_server ) then:
  - publishes "connected"+uniqueID to the [root topic] ( using topic ) 
  - subscribes to the topic "[root topic]/composeClientID()/in"  with a callback to handle
  - If the first character of the topic "[root topic]/composeClientID()/in" is an 1, 
    switch ON the ESP Led, else switch it off

  - after a delay of "[root topic]/composeClientID()/in" minimum, it will publish 
    a composed payload to 
  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. 
  
*/

const char* topic  = "cat-detector";
long timeBetweenMessages = 500; //1000 * 20 * 1;

#ifdef IS_WEMOS
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}


String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

String composeClientID() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String clientId;
  clientId += "esp-";
  clientId += macToStr(mac);
  return clientId;
}

void reconnect() {
  // Loop until we're reconnected
  for (int i = 0; i < MQTT_MAX_CONNECT_ATTEMPTS; i++) {
    if (client.connected()) {
      break;
    }
    Serial.print("Attempting MQTT connection...");

    String clientId = composeClientID() ;
    clientId += "-";
    clientId += String(micros() & 0xff, 16); // to randomise. sort of

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(topic, ("connected " + composeClientID()).c_str() , true );
      // ... and resubscribe
      // topic + clientID + in
      String subscription;
      subscription += topic;
      client.subscribe(subscription.c_str() );
      Serial.print("subscribed to : ");
      Serial.println(subscription);
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.print(" wifi=");
      Serial.print(WiFi.status());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setupMqtt() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void transmitMqtt(String payload) {
  // confirm still connected to mqtt server
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > timeBetweenMessages ) {
    lastMsg = now;
    payload += " ";
    payload += composeClientID();
    String pubTopic;
    pubTopic += topic;
    Serial.print("Publish topic: ");
    Serial.println(pubTopic);
    Serial.print("Publish message: ");
    Serial.println(payload);
    client.publish((char *)pubTopic.c_str(), (char *)payload.c_str(), false);
  }
}
#endif