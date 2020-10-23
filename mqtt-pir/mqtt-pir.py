#!/usr/bin/env python3
# coding: utf8

import time
from threading import Timer

import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO

import config


class ResetTimer:
    """Timer that will restart as soon as start is called again"""

    def __init__(self, interval, func):
        self.func = func
        self.interval = interval
        self.running = False

    def callback(self):
        self.running = False
        self.func()

    def start(self):
        if self.running is True:
            self.timer.cancel()
        self.timer = Timer(self.interval, self.callback)
        self.timer.start()
        self.running = True


def on_connect(client, userdata, flags, rc):
    print("Connected with result code ", str(rc))
    client.publish(config.AVAILABILITY_TOPIC, "online", 1, True)


def timeout():
    if GPIO.input(config.PIR_GPIO_PIN) is 1:
        print("Timeout entered, pin still up. Restarting timer")
        timer.start()
    else:
        print("Timeout entered and resetted to 0")
        client.publish(config.MOVEMENT_TOPIC, 0, 1, retain=True)


def on_pir_rised(channel):
    """Callback when PIR pin has raised. Publish 1 and start timer to fall back to 0."""
    print("Movement detected")
    client.publish(config.MOVEMENT_TOPIC, 1, 1, retain=True)
    timer.start()


# Setup MQTT connection
client = mqtt.Client()
client.on_connect = on_connect
client.will_set(config.AVAILABILITY_TOPIC, "offline", 1, True)
client.connect(config.MQTT_BROKER_HOSTNAME, config.MQTT_BROKER_PORT, 60)
client.loop_start()

# Setup GPIO PIR pins
GPIO.setmode(GPIO.BOARD)
GPIO.setup(config.PIR_GPIO_PIN, GPIO.IN)
GPIO.add_event_detect(config.PIR_GPIO_PIN, GPIO.RISING, callback=on_pir_rised)

# Publish initial state and start timer initially
initial_state = GPIO.input(config.PIR_GPIO_PIN)
print("Initial movement " + str(initial_state))
client.publish(config.MOVEMENT_TOPIC, initial_state, 1, retain=True)
timer = ResetTimer(config.MOVEMENT_TIMEOUT_SECONDS, timeout)

while True:
    time.sleep(2)
