#!/usr/bin/env python3
# coding: utf8

import time
import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt
from threading import Timer

MOVEMENT_TOPIC = "room/movement"
MQTT_BROKER_HOSTNAME = "192.168.0.xxx"
MQTT_BROKER_PORT = "1883"
PIR_GPIO_PIN = 11

# When it should drop back to false after the last movement
MOVEMENT_TIMEOUT_SECONDS = 120.0


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


def on_connect(client, userdata, flags, rc):
    print("Connected with result code ", str(rc))


def timeout():
    print("timeout callback entered")
    if GPIO.input(PIR_GPIO_PIN) is 1:
        timer.start()
    else:
        client.publish(MOVEMENT_TOPIC, 0, 1, retain=True)


timer = ResetTimer(MOVEMENT_TIMEOUT_SECONDS, timeout)

client = mqtt.Client()
client.on_connect = on_connect
client.connect(MQTT_BROKER_HOSTNAME, MQTT_BROKER_PORT, 60)

client.loop_start()

GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIR_GPIO_PIN, GPIO.IN)


def on_pir_rised(channel):
    print("PIR rised")
    client.publish(MOVEMENT_TOPIC, 1, 1, retain=True)
    timer.start()


GPIO.add_event_detect(PIR_GPIO_PIN, GPIO.RISING, callback=on_pir_rised)

print("Bewegung " + str(GPIO.input(PIR_GPIO_PIN)))
client.publish(MOVEMENT_TOPIC, GPIO.input(PIR_GPIO_PIN), 1, retain=True)

while True:
    time.sleep(2)
