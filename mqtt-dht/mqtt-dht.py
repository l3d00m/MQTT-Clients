#!/usr/bin/env python3

# Modified from https://github.com/corbanmailloux/esp-mqtt-dht
import paho.mqtt.client as mqtt
import time
import Adafruit_DHT
import config


# The callback for when the client receives a CONNACK response from the server.
# noinspection PyUnusedLocal
def on_connect(client, userdata, flags, rc):
    if rc > 0:
        raise ConnectionError('Wrong result code from mqtt server {}'.format(rc))
    print("Connected with result code {}".format(rc))


client = mqtt.Client()
client.on_connect = on_connect
client.will_set(config.AVAILABILITY_TOPIC, "offline", 1, True)
client.connect(config.MQTT_BROKER_HOSTNAME,
               config.MQTT_BROKER_PORT,
               10)
client.loop_start()

total_count = 0
last_successful = 0

while True:
    time.sleep(config.SECONDS_TO_SLEEP)
    total_count += 1

    # Publish an offline message if the current value is dated, i.e. the reading failed often
    if total_count - last_successful > 20:
        print("The last 20 readings were unsuccessful, now displaying as offline")
        client.publish(config.AVAILABILITY_TOPIC, "offline", 1, True)

    # Read the values
    humidity, temperature = Adafruit_DHT.read_retry(22, config.DHT_PIN)

    # Verify that the reading worked
    if humidity is None or temperature is None:
        print('Failed to get reading. Skipping ...')
        continue

    # Round the values because their resolution isn't that high anyway
    humidity = round(humidity, 1)
    temperature = round(temperature, 1)

    # Out of bounds validity check
    if (humidity < 0) or (humidity > 100) or (temperature < -20) or (temperature > 50):
        print("Value is out of bounds, skipping this reading...")
        continue

    # We only get to these lines if the reading was successful
    #  Set state to online in case it was offline
    client.publish(config.AVAILABILITY_TOPIC, "online", 1, True)
    
    client.publish(config.HUMIDITY_TOPIC, humidity, retain=True)
    print('Published new humidity')

    client.publish(config.TEMPERATURE_TOPIC, temperature, retain=True)
    print('Published new temperature')

    # (re)set loop values
    last_successful = total_count
    skipped_count = 0
