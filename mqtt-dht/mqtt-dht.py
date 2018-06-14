#!/usr/bin/env python3

# Modified from https://github.com/corbanmailloux/esp-mqtt-dht
import paho.mqtt.client as mqtt
import time
import Adafruit_DHT

TEMPERATURE_TOPIC = "room/temperature"
HUMIDITY_TOPIC = "room/humidity"
MQTT_BROKER_HOSTNAME = "192.168.0.xxx"
MQTT_BROKER_PORT = "1883"
DHT_PIN = 4
DHT_VERSION = Adafruit_DHT.DHT22


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    if rc > 0:
        raise ConnectionError('Wrong result code from mqtt server {}'.format(rc))
    print("Connected with result code {}".format(rc))


client = mqtt.Client()
client.on_connect = on_connect
client.connect(MQTT_BROKER_HOSTNAME,
               MQTT_BROKER_PORT,
               10)
client.loop_start()
last_humidity = False
last_temperature = False
skipped_count = 0
while True:
    time.sleep(3)
    humidity, temperature = Adafruit_DHT.read_retry(DHT_VERSION, DHT_PIN)
    if humidity is not None and temperature is not None:
        # Basic validity check
        if (humidity < 1) or (humidity > 99) or (temperature < 0) or (temperature > 50):
            print("Value out of bounds, skipping...")
            continue
        # More advanced validity check
        if last_temperature is not False and last_humidity is not False:
            # If this is not the first run, check that the difference between two measurements is not too high
            if abs(last_humidity - humidity) > 5 or abs(last_temperature - temperature > 2):
                if skipped_count > 5:
                    # If somehow the script hasn't run for some time or started with a wrong value, this is a fail safe
                    # It should be very rarely land here
                    print("We already skipped the last ten, now assuming that this is the right offset")
                else:
                    print("Difference to last value is too high, skipping...")
                    skipped_count += 1
                    continue

        skipped_count = 0
        last_humidity = humidity
        last_temperature = temperature

        client.publish(TEMPERATURE_TOPIC, round(temperature, 1))
        client.publish(HUMIDITY_TOPIC, round(humidity, 1))
        print('Published. Sleeping ...')
    else:
        print('Failed to get reading. Skipping ...')
