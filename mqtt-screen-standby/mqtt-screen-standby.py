#!/usr/bin/env python3
import paho.mqtt.client as mqtt
import os
import subprocess


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, rc, *extra_params):
    print('Connected with result code ' + str(rc))
    # Subscribing to receive RPC requests
    client.subscribe('room/monitor/cmnd')
    client.publish('room/monitor/stat', get_state(), 1)


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print('Topic: ' + msg.topic + '\nMessage: ' + str(msg.payload))
    # Decode JSON request
    data = msg.payload.decode("utf-8")
    # Check request method
    if data == 'ON':
        os.system('vcgencmd display_power 1')
    if data == 'OFF':
        os.system('vcgencmd display_power 0')
    client.publish('room/monitor/stat', get_state(), 1)


def get_state():
    output = subprocess.check_output("vcgencmd display_power", shell=True).decode("utf-8")
    if "display_power=0" in output:
        return 'OFF'
    if "display_power=1" in output:
        return 'ON'


client = mqtt.Client()
# Register connect callback
client.on_connect = on_connect
# Registed publish message callback
client.on_message = on_message
client.connect("192.168.0.40", 1883, 20)

client.loop_forever()
