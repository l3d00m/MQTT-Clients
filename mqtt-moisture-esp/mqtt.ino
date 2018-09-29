void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  // Loop until we're connected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(host_name, availability_topic, 1, true, "offline")) {
      Serial.println("connected");
      client.publish(availability_topic, "online", true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publish_moisture(int moisture) {
  char convert[4];
  itoa(moisture, convert, 10);
  char* payload = convert;
  Serial.print("Current moisture that will be published: ");
  Serial.println(convert);
  client.publish(stat_topic, convert, true);
}
