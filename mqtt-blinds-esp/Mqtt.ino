void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // Loop until we're connected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(hostname, availability_topic, 1, true, "offline")) {
      Serial.println("connected");
      client.publish(availability_topic, "online", true);
      client.subscribe(cmnd_topic);
      client.subscribe(pos_topic);
      client.subscribe(speed_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
