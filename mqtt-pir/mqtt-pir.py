#!/usr/bin/env python3
#coding: utf8

import time
import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt
from threading import Timer

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
    print("Connected with result code " , str(rc))

def timeout():
    print("timeout callback entered")
    if GPIO.input(11) is 1:
        timer.start()
    else:
        client.publish("room/bewegung/stat", 0, 1, retain=True)

timer = ResetTimer(120.0, timeout);

client = mqtt.Client();
client.on_connect = on_connect
client.connect("192.168.0.40", 1883, 60)

client.loop_start()

GPIO.setmode(GPIO.BOARD)
GPIO.setup(11, GPIO.IN)

def on_pir_rised(channel):
    print("PIR rised")
    client.publish("room/bewegung/stat", 1, 1, retain=True)
    timer.start()


GPIO.add_event_detect(11, GPIO.RISING, callback = on_pir_rised)

print("Bewegung " + str(GPIO.input(11)))
client.publish("room/bewegung/stat", GPIO.input(11), 1, retain=True)

while True:
    time.sleep(2)


