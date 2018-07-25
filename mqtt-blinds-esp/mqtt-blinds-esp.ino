#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#include "Constants.h"

// Max Steps (i.e. how far to move); gets overriden by mqtt config command
int STEPS = 23000;

/* Global values used in the program */
int custom_rpm = -1;
int MOVE_TO = -2;
int current_steps = 0;
int prev_steps = -1;
unsigned long previousMicros = 0;
// Delay between each motor step; gets overriden by mqtt config command
int rpm;

WiFiClient espClient;
PubSubClient client(espClient);
const int stepsPerRotation = 4076; // 64 steps per revolution * 63,8... gear reduction ratio: https://42bots.com/tutorials/28byj-48-stepper-motor-with-uln2003-driver-and-arduino-uno/

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(200);
  setup_motor();
  turn_off_motor();
  loadCurrentStepsFromEeprom();
  loadDefaultStepsFromEeprom();
  loadSpeedFromEeprom();
  myStepper.setSpeed(60);
}

void callback(char* tpc, byte* payload_bytes, unsigned int length) {
  char payload[length];
  for (int i = 0; i < length; i++) {
    payload[i] = (char)payload_bytes[i];
  }
  String cmnd((char*)payload);
  String topic((char*)tpc);
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
      MOVE_TO = STEPS;
      Serial.println("Will move down");
    } else if (cmnd.startsWith("STOP")) {
      finish_moving();
    }
  } else if (topic.equals(next_speed_topic)) {
    Serial.print("Received custom speed for next cmnd: ");
    Serial.println(cmnd.toInt());
    custom_rpm = cmnd.toInt();
  } else if (topic.equals(speed_config_topic)) {
    Serial.print("Received speed config: ");
    Serial.println(cmnd.toInt());
    rpm = cmnd.toInt();
    writeSpeedToEeprom();
  } else if (topic.equals(steps_config_topic)) {
    Serial.print("Received steps config: ");
    Serial.println(cmnd.toInt());
    STEPS = cmnd.toInt();
    writeDefaultStepsToEeprom();
    // move to the new limit to make it easier to configure
    MOVE_TO = STEPS;
  } else {
    Serial.println("Topic unknown, nothing done. This should not happen");
  }
}

void finish_moving() {
  MOVE_TO = -1;
  custom_rpm = -1;
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
    // convert rotations per minute to step delay in microseconds
    // and use custom_rpm if set, else usual rpm
    int step_delay = 60L * 1000L * 1000L / stepsPerRotation / ((custom_rpm > 0) ? custom_rpm : rpm);
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

      previousMicros = currentMicros;
      prev_steps = current_steps;


      if (current_steps <= MOVE_TO) {
        // Is currently moving down
        current_steps ++;
      } else if (current_steps > MOVE_TO) {
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
  if (steps > STEPS) {
    // This can happen if the new max get changed, don't error
    return 100;
  }
  return (100 - round(((double)steps / STEPS) * 100));
}

int percentageToSteps(int percentage) {
  if (percentage < 0 || percentage > 100) {
    Serial.println("[ERROR] We have a problem with percentage here, was ");
    Serial.println(percentage);
    return 0;
  }
  return round(STEPS * ((double)(100 - percentage) / 100));
}

void publishCurrentPercentage() {
  int num = (int) stepsToPercentage(current_steps);
  char convert[3];
  itoa(num, convert, 10);
  char* payload = convert;
  Serial.print("Current percentage, will be published: ");
  Serial.println(convert);
  client.publish(stat_topic, convert, true);
}

