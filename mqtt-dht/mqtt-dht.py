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
    client.publish(config.AVAILABILITY_TOPIC, "online", 1, True)


client = mqtt.Client()
client.on_connect = on_connect
client.will_set(config.AVAILABILITY_TOPIC, "offline", 1, True)
client.connect(config.MQTT_BROKER_HOSTNAME,
               config.MQTT_BROKER_PORT,
               10)
client.loop_start()
last_humidity = False
last_temperature = False
skipped_count = 0
while True:
    time.sleep(config.SECONDS_TO_SLEEP)
    humidity, temperature = Adafruit_DHT.read_retry(22, config.DHT_PIN)
    # Verify that the reading worked
    if humidity is None or temperature is None:
        print('Failed to get reading. Skipping ...')
        continue
    # Basic validity check
    if (humidity < 1) or (humidity > 99) or (temperature < 0) or (temperature > 50):
        print("Value is out of bounds, skipping this reading...")
        continue
    # More advanced validity check
    if last_temperature is not False and last_humidity is not False:
        # If this is not the first run, check that the difference between two measurements is not too high
        if abs(last_humidity - humidity) >= 2.5 or abs(last_temperature - temperature >= 1):
            if skipped_count < 15:
                print("Difference to last value is too high, skipping for the " + str(skipped_count) + ". time...")
                skipped_count += 1
                continue
            else:
                # If somehow the script hasn't run for some time or started with a wrong value, this is a fail safe
                # It should very rarely land here
                print("We already skipped a lot of values, now assuming that this is the right value")

    if abs(last_humidity - humidity) >= 0.2:
        client.publish(config.HUMIDITY_TOPIC, round(humidity, 1), retain=True)
        print('Published new humidity')
        last_humidity = humidity
    if last_temperature != temperature:
        client.publish(config.TEMPERATURE_TOPIC, round(temperature, 1), retain=True)
        print('Published new temperature')
        last_temperature = temperature

    skipped_count = 0
