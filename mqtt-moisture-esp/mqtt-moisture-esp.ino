#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#include "Constants.h"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  delay(200);
  initOTA();
  pinMode(OUTPUT_PIN, OUTPUT);
}

unsigned long previous_millis = 0;
int last_moist = -1;

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  ArduinoOTA.handle();
  if (!client.connected()) {
    setup_mqtt();
  }
  client.loop();
  unsigned long current_millis = millis();
  if (current_millis - previous_millis >= READ_DELAY * 1000) { // Custom non blocking delay
    digitalWrite(OUTPUT_PIN, HIGH);
    delay(100);
    int moist = analogRead(A0);
    delay(100);
    digitalWrite(OUTPUT_PIN, LOW);
    Serial.println(moist);
    if (last_moist != moist) {
      publish_moisture(moist);
      last_moist = moist;
    }
    previous_millis = current_millis;
  }
}
