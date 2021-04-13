#!/usr/bin/env python3
import json
import os
import subprocess
import time
import re

import paho.mqtt.client as mqtt
from pms5003 import PMS5003

import config

# Configure the PMS5003
pms5003 = PMS5003(
    device='/dev/ttyAMA0',
    baudrate=9600,
    pin_enable=config.ENABLE_PIN,
    pin_reset=config.RESET_PIN
)


def on_connect(client, userdata, rc, *extra_params):
    # The callback for when the client receives a CONNACK response from the server.
    print('Connected with result code ' + str(rc))


def get_state():
    try:
        data = pms5003.read()
        result = {}
        result['pm1'] = data.pm_ug_per_m3(1.0)
        result['pm2_5'] = data.pm_ug_per_m3(2.5)
        result['pm10'] = data.pm_ug_per_m3(10)
        return result
    except Exception as err:
        print('Error reading:', err)
        return None


def publish_hass_discovery():
    for pm_type in ['pm1', 'pm2_5', 'pm10']:
        config_dict = {}
        config_dict['name'] = config.DEVICE_NAME + pm_type
        config_dict['state_topic'] = config.STATE_TOPIC
        config_dict['unit_of_measurement'] = "ug/m3"
        config_dict['value_template'] = "{{value_json." + pm_type + "}}"
        mqtt_name = re.sub(r'\W+', '', config_dict['name'], flags=re.ASCII)
        client.publish(f"homeassistant/sensor/pms5003/{mqtt_name}/config", json.dumps(config_dict), retain=True)


client = mqtt.Client()
client.on_connect = on_connect
client.will_set(config.AVAILABILITY_TOPIC, "offline", 1, True)
client.connect(config.MQTT_BROKER_HOST, config.MQTT_BROKER_PORT, 20)
client.loop_start()

publish_hass_discovery()

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
    result = get_state()
    # and verify that the reading worked
    if result is None:
        continue

    # We only get to these lines if the reading was successful
    #  Set state to online in case it was offline
    client.publish(config.AVAILABILITY_TOPIC, "online", 1, True)

    client.publish(config.STATE_TOPIC, json.dumps(result), retain=True)
    print('Published new result')

    # (re)set loop values
    last_successful = total_count
    skipped_count = 0
