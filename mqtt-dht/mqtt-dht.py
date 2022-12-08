#!/usr/bin/env python3

# Modified from https://github.com/corbanmailloux/esp-mqtt-dht
import paho.mqtt.client as mqtt
import time
import adafruit_dht
import board
import config
import json
import re


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

dhtDevice = adafruit_dht.DHT22(board.D4)

discovery_temp = {}
discovery_temp['name'] = config.HASS_NAME_TEMPERATURE
discovery_temp['state_topic'] = config.TEMPERATURE_TOPIC
discovery_temp['device_class'] = 'temperature'
discovery_temp['unit_of_meas'] = '°C'
discovery_temp['avty_t'] = config.AVAILABILITY_TOPIC
discovery_temp_json = json.dumps(discovery_temp)
unique_temp_id = ''.join([i for i in config.TEMPERATURE_TOPIC if i.isalpha()])
discovery_temp_topic = "homeassistant/sensor/" + unique_temp_id  + "/config"

discovery_hum = {}
discovery_hum['name'] = config.HASS_NAME_HUMIDITY
discovery_hum['state_topic'] = config.HUMIDITY_TOPIC
discovery_hum['device_class'] = 'humidity'
discovery_hum['unit_of_meas'] = '%'
discovery_hum['avty_t'] = config.AVAILABILITY_TOPIC
discovery_hum_json = json.dumps(discovery_hum)
unique_hum_id = ''.join([i for i in config.HUMIDITY_TOPIC if i.isalpha()])
discovery_hum_topic = "homeassistant/sensor/" + unique_hum_id  + "/config"



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
    try:
        temperature = dhtDevice.temperature
        humidity = dhtDevice.humidity
    except RuntimeError as error:
        print(error.args[0])
        continue

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
    client.publish(discovery_temp_topic, discovery_temp_json, retain=True)
    client.publish(discovery_hum_topic, discovery_hum_json, retain=True)

    client.publish(config.HUMIDITY_TOPIC, humidity, retain=True)
    print('Published new humidity')

    client.publish(config.TEMPERATURE_TOPIC, temperature, retain=True)
    print('Published new temperature')

    # (re)set loop values
    last_successful = total_count
    skipped_count = 0
