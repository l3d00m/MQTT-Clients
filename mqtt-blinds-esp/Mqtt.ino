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
// Where the window sensor reports where it's closed or not
#define window_state_topic "thomas/window/contact"

void setup_mqtt() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  // Loop until we're connected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientname, availability_topic, 1, true, "offline")) {
      Serial.println("connected to MQTT server");
      client.publish(availability_topic, "online", true);
      client.subscribe(cmnd_topic);
      client.subscribe(pos_topic);
      client.subscribe(next_speed_topic);
      client.subscribe(speed_config_topic);
      client.subscribe(steps_config_topic);
      client.subscribe(speed_down_config_topic);
    } else {
      Serial.print("failed with code ");
      Serial.print(client.state());
      Serial.println(", trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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
    } else if (cmnd.startsWith("TOGGLE")) {
      if (current_steps == 0 || (MOVE_TO >= 0 && current_steps > MOVE_TO)) {
        // If blinds are currently open or in the process of opening, close them
        MOVE_TO = MAX_STEPS;
        Serial.println("Toggling, will close now");
      } else {
        // otherwise simply open them
        MOVE_TO = 0;
        Serial.println("Toggling, will open now");
      }
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

void publishCurrentPercentage() {
  int num = (int) stepsToPercentage(current_steps);
  char convert[3];
  itoa(num, convert, 10);
  char* payload = convert;
  Serial.print("Current percentage, will be published: ");
  Serial.println(convert);
  client.publish(stat_topic, convert, true);
}

void publishWindowState(int window_state){
    char convert[1];
    itoa(window_state, convert, 10);
    char* payload = convert;
    client.publish(window_state_topic, payload, true);
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
