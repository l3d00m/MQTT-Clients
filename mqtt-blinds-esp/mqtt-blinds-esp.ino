#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#include "Constants.h"

// Where to receive commands
#define cmnd_topic                topic_prefix "cmnd"
// Where the current state (as percentage) get's published to
#define stat_topic                topic_prefix "stat"
// To manually set the blinds' position
#define pos_topic                 topic_prefix "pos"
// To override the speed (ie rpm) for the next command
#define next_speed_topic          topic_prefix "next_speed"
// Availibity (via MQTT will)
#define availability_topic        topic_prefix "available"
// To configure the speed (only up if down configured too)
#define speed_config_topic        topic_prefix "config/speed"
// Optionally configure a seperate speed down
#define speed_down_config_topic   topic_prefix "config/speed_down"
// To configure the total steps (i.e. how far the blinds will go)
#define steps_config_topic        topic_prefix "config/steps"

// Max Steps (i.e. how far to move); gets overriden by mqtt config command
int MAX_STEPS;

/* Global values used in the program */
int temporary_rpm = -1;
int MOVE_TO = -2;
int current_steps = 0;
int prev_steps = -1;
unsigned long previousMicros = 0;
int up_rpm; // Delay between each motor step; gets overriden by mqtt config command
int down_rpm; // Delay between motor steps while moving down, is optional

WiFiClient espClient;
PubSubClient client(espClient);
const int stepsPerRotation = 4076; // 64 steps per revolution * 63,8... gear reduction ratio: https://42bots.com/tutorials/28byj-48-stepper-motor-with-uln2003-driver-and-arduino-uno/

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(200);
  setup_motor();
  turn_off_motor();
  initOTA();
  loadMaxStepsFromEeprom(); // load max steps before current steps for validity checks in current steps
  loadCurrentStepsFromEeprom();
  loadUpSpeedFromEeprom();
  loadDownSpeedFromEeprom(); // load down speed after normal speed to calculate it if not set
}

void callback(char* tpc, byte* payload_bytes, unsigned int length) {
  if (length == 0) {
    Serial.println("Received MQTT command with empty payload, aborting...");
    return;
  }
  char payload[length];
  for (int i = 0; i < length; i++) {
    payload[i] = (char)payload_bytes[i];
  }
  String cmnd((char*)payload);
  String topic(tpc);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(cmnd);

  if (topic.equals(pos_topic)) {
    Serial.print("Moving to percentage ");
    int percentage = cmnd.toInt();
    Serial.print(percentage);
    if (percentage >= 0 || percentage <= 100) {
      MOVE_TO = percentageToSteps(percentage);
      Serial.print(" which equals steps: ");
      Serial.println(MOVE_TO);
    } else {
      Serial.println(" ... welp we aren't moving, this is not a valid percentage");
    }
  } else if (topic.equals(cmnd_topic)) {
    if (cmnd.startsWith("OPEN")) {
      MOVE_TO = 0;
      Serial.println("Will move up");
    } else if (cmnd.startsWith("CLOSE")) {
      MOVE_TO = MAX_STEPS;
      Serial.println("Will move down");
    } else if (cmnd.startsWith("STOP")) {
      Serial.println("Stopping");
      finish_moving();
    }
  } else if (topic.equals(next_speed_topic)) {
    Serial.print("Received temporary speed only for next cmnd: ");
    Serial.println(cmnd.toInt());
    temporary_rpm = cmnd.toInt();
  } else if (topic.equals(speed_config_topic)) {
    Serial.print("Received speed for moving up: ");
    Serial.println(cmnd.toInt());
    up_rpm = cmnd.toInt();
    writeUpSpeedToEeprom();
  } else if (topic.equals(speed_down_config_topic)) {
    Serial.print("Received speed for moving down config: ");
    Serial.println(cmnd.toInt());
    down_rpm = cmnd.toInt();
    writeDownSpeedToEeprom();
  } else if (topic.equals(steps_config_topic)) {
    Serial.print("Received max steps config: ");
    Serial.println(cmnd.toInt());
    MAX_STEPS = cmnd.toInt();
    writeMaxStepsToEeprom();
    // move to the new limit to make it easier to configure
    MOVE_TO = MAX_STEPS;
  } else {
    Serial.println("Topic unknown, nothing done. This should not happen");
  }
}

void finish_moving() {
  MOVE_TO = -1; // set to -1, which will leave the motor on for a few ms before shutting down
  temporary_rpm = -1; // reset next speed rpm
  publishCurrentPercentage();
  writeCurrentStepsToEeprom();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    turn_off_motor();
    setup_wifi();
  }
  ArduinoOTA.handle();
  if (!client.connected()) {
    turn_off_motor();
    setup_mqtt();
    publishCurrentPercentage();
  }
  client.loop();
  unsigned long currentMicros = micros();
  if (MOVE_TO >= 0) {
    // Sets true if moving down, else false
    bool moving_down = current_steps <= MOVE_TO;

    // Select which RPM to use based on whether we are moving down or up or if a temporary rpm is set
    int rpm = moving_down ? down_rpm : up_rpm;
    if (temporary_rpm > 0) {
      rpm = temporary_rpm; // overwrite
    }
    // convert rotations per minute to step delay in microseconds with the correct rpm to use
    int step_delay = 90 * 1000 * 1000L / stepsPerRotation / rpm;

    if (current_steps == MOVE_TO) {
      if (prev_steps == -1) {
        Serial.println("First run finished");
      } else if (prev_steps < MOVE_TO) {
        Serial.println("Finished moving down");
      } else if (prev_steps > MOVE_TO) {
        Serial.println("Finished moving up");
      }
      finish_moving();
    } else if (currentMicros - previousMicros >= step_delay) { // Custom non blocking delay
      if (stepsToPercentage(current_steps) != stepsToPercentage(prev_steps)) {
        publishCurrentPercentage();
      }

      step(current_steps);

      previousMicros = currentMicros;
      prev_steps = current_steps;

      if (moving_down) {
        // Is currently moving down
        current_steps ++;
      } else {
        // Is currently moving up
        current_steps --;
      }
    }
  } else if (MOVE_TO == -1) {
    // MOVE_TO = -1 means the motor has just stopped operation and should still hold to avoid blinds falling down
    if (currentMicros - previousMicros >= HOLD_DELAY_MS * 1000) { // Custom non blocking delay
      MOVE_TO = -2;
      Serial.println("Hold finished");
    }
  } else {
    // finally turn off motor
    turn_off_motor();
  }
}

byte stepsToPercentage(int steps) {
  if (steps < -1) {
    Serial.print("[ERROR] We have a problem with steps here, was ");
    Serial.println(steps);
    return 0;
  }
  if (steps > MAX_STEPS) {
    // This can happen if the new max get changed, don't error
    return 100;
  }
  return (100 - round(((double)steps / MAX_STEPS) * 100));
}

int percentageToSteps(int percentage) {
  if (percentage < 0 || percentage > 100) {
    Serial.println("[ERROR] We have a problem with percentage here, was ");
    Serial.println(percentage);
    return 0;
  }
  return round(MAX_STEPS * ((double)(100 - percentage) / 100));
}

void publishCurrentPercentage() {
  int num = (int) stepsToPercentage(current_steps);
  char convert[3];
  itoa(num, convert, 10);
  char* payload = convert;
  Serial.print("Current percentage, will be published: ");
  Serial.println(convert);
  client.publish(stat_topic, convert, true);
  if (num % 10 == 0) {
    // Write steps to eeprom every 10 percent to avoid complete position loss on power off, but not every percentage to save eeprom lifetime
    writeCurrentStepsToEeprom();
  }
}
