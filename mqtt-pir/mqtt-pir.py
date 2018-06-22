#!/usr/bin/env python3
# coding: utf8

import time
import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt
from threading import Timer
import config


class ResetTimer:
    def __init__(self, interval, func):
        self.func = func
        self.interval = interval
        self.running = False

    def callback(self):
        self.running = False;
        self.func()

    def start(self):
        if self.running is True:
            self.timer.cancel()
        self.timer = Timer(self.interval, self.callback)
        self.timer.start()
        self.running = True

    def stop(self):
        if self.running is True:
            self.timer.cancel()
            self.running = False


# noinspection PyUnusedLocal
def on_connect(client, userdata, flags, rc):
    print("Connected with result code ", str(rc))
    client.publish(config.AVAILABILITY_TOPIC, "online", 1, True)


def timeout():
    print("timeout callback entered")
    if GPIO.input(config.PIR_GPIO_PIN) is 1:
        timer.start()
    else:
        client.publish(config.MOVEMENT_TOPIC, 0, 1, retain=True)


timer = ResetTimer(config.MOVEMENT_TIMEOUT_SECONDS, timeout)

client = mqtt.Client()
client.on_connect = on_connect
client.will_set(config.AVAILABILITY_TOPIC, "offline", 1, True)
client.connect(config.MQTT_BROKER_HOSTNAME, config.MQTT_BROKER_PORT, 60)

client.loop_start()

GPIO.setmode(GPIO.BOARD)
GPIO.setup(config.PIR_GPIO_PIN, GPIO.IN)


# noinspection PyUnusedLocal
def on_pir_rised(channel):
    print("PIR rised")
    client.publish(config.MOVEMENT_TOPIC, 1, 1, retain=True)
    timer.start()


GPIO.add_event_detect(config.PIR_GPIO_PIN, GPIO.RISING, callback=on_pir_rised)

print("Movement " + str(GPIO.input(config.PIR_GPIO_PIN)))
client.publish(config.MOVEMENT_TOPIC, GPIO.input(config.PIR_GPIO_PIN), 1, retain=True)

while True:
    time.sleep(2)
