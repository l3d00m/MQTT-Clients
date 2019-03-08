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
