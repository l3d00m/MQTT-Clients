#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

#include "Constants.h"

#define steps_per_rotation 4076 // 64 steps per revolution * 63,8... gear reduction ratio: https://42bots.com/tutorials/28byj-48-stepper-motor-with-uln2003-driver-and-arduino-uno/

// Max Steps (i.e. how far to move)
int MAX_STEPS;

/* Global values used in the program */
int temporary_rpm = -1;
int MOVE_TO = -2;
int current_steps = 0;
int prev_steps = -1;
unsigned long previousMicros = 0;
int up_rpm; // Delay between each motor step while moving up
int down_rpm; // Delay between motor steps while moving down

WiFiClient espClient;
PubSubClient client(espClient);

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

void finish_moving() {
  MOVE_TO = -1; // set to -1, which will leave the motor on for a few ms before shutting down
  temporary_rpm = -1; // reset next speed rpm
  publishCurrentPercentage();
  writeCurrentStepsToEeprom();
}

int getRPM(bool moving_down) {
  // Select which RPM to use based on whether we are moving down or up or if a temporary rpm is set
  int rpm = moving_down ? down_rpm : up_rpm;
  if (temporary_rpm > 0) {
    rpm = temporary_rpm; // overwrite
  }
  return rpm;
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

    // convert rotations per minute to step delay in microseconds with the correct rpm to use
    int step_delay = 90 * 1000 * 1000L / steps_per_rotation / getRPM(moving_down); // this is not real RPM value, conversiona are incorrect currently

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
      step(current_steps);
      
      if (stepsToPercentage(current_steps) != stepsToPercentage(prev_steps)) { // publish and save percentage on change
        publishCurrentPercentage();
        if (stepsToPercentage(current_steps) % 10 == 0) {
          // Write steps to eeprom every 10 percent to avoid complete position loss on power off, but not every percentage to save eeprom lifetime
          writeCurrentStepsToEeprom();
        }
      }

      previousMicros = currentMicros;
      prev_steps = current_steps;

      if (moving_down) {
        current_steps ++;
      } else {
        current_steps --;
      }
    }
  } else if (MOVE_TO == -1) {
    // MOVE_TO = -1 means the motor has just stopped operation and should still hold to avoid blinds falling down
    if (currentMicros - previousMicros >= 200 * 1000) { // Custom non blocking delay
      MOVE_TO = -2;
      Serial.println("Hold finished");
    }
  } else {
    // finally turn off motor
    turn_off_motor();
  }
}
