1\) Create a file called `config.py` with the following contents:

```python
STATE_TOPIC = 'pms5003/room1/state'
MQTT_BROKER_PORT = 1883
MQTT_BROKER_HOST = "192.168.xxx.xxx"
AVAILABILITY_TOPIC = "pms5003/room1/available"
ENABLE_PIN = 24  # optional
RESET_PIN = 23  # optional
SECONDS_TO_SLEEP = 2
DEVICE_NAME = 'My Name'  # Shown in homeassistant, use a different name for each device
```

2\) Install the required dependencies:

1. Install python3 and pip if not already installed: `sudo apt install python3 python3-pip`
1. Execute the following command in this folder to install the python dependencies: `sudo pip3 install -r requirements.txt`

3\) To let the script run as a service on debian:

1.  Symlink the service file from this directory to `/etc/systemd/system/mqtt-pms5003.service`, e.g. `sudo ln -sf /home/pi/MQTT-Clients/mqtt-pms5003/mqtt-pms5003.service /etc/systemd/system/mqtt-pms5003.service`
2.  Reload the service configuration with `sudo systemctl daemon-reload`
3.  Enable (= autostart) the service with `sudo systemctl enable mqtt-pms5003`
4.  Run it with `sudo systemctl start mqtt-pms5003`

4\) It'll be added to Homeassistant automatically via discovery
