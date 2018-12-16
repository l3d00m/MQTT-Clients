1\) Create a file called `config.py` with the following contents:

```python
TEMPERATURE_TOPIC = "room/climate/temperature"
HUMIDITY_TOPIC = "room/climate/humidity"
AVAILABILITY_TOPIC = "room/climate/available"
MQTT_BROKER_HOSTNAME = "192.168.0.xxx"
MQTT_BROKER_PORT = 1883
DHT_PIN = 4
SECONDS_TO_SLEEP = 4
```

2\) Install the required dependencies:
 1. Install python3 and pip if not already installed: `sudo apt install python3 python3-pip`
 1. Install the library for the DHT sensor by executing this in your home folder `git clone https://github.com/adafruit/Adafruit_Python_DHT.git && cd Adafruit_Python_DHT && sudo python3 setup.py install`
 1. Execute the following command in this folder to install the python dependencies: `pip3 install -r requirements.txt`

3\) Make the script run as a service on debian:
 1. Symlink the service file from this directory to `/etc/systemd/system/mqtt-dht.service`, e.g. `sudo ln -sf /home/pi/MQTT-Clients/mqtt-dht/mqtt-dht.service /etc/systemd/system/mqtt-dht.service`
 2. Reload the service configuration with `sudo systemctl daemon-reload`
 3. Enable (= autostart) the service with `sudo systemctl enable mqtt-dht` 
 4. Run it with `sudo systemctl start mqtt-dht`

4\) Add this to your homeassistant configuration:
 
```yaml
sensor:
  - platform: mqtt
    name: "Temperature"
    state_topic: "room/climate/temperature"
    unit_of_measurement: '°C'
    availability_topic: "room/climate/status"
    payload_available: "online"
    payload_not_available: "offline"
    device_class: temperature
  - platform: mqtt
    name: "Humidity"
    state_topic: "room/climate/humidity"
    unit_of_measurement: '%'
    availability_topic: "room/climate/available"
    payload_available: "online"
    payload_not_available: "offline"
    device_class: humidity
```
