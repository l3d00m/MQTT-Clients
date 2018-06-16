1\) Create a file called `config.py` with the following contents:

```python
TEMPERATURE_TOPIC = "room/climate/temperature"
HUMIDITY_TOPIC = "room/climate/humidity"
AVAILABILITY_TOPIC = "room/climate/status"
MQTT_BROKER_HOSTNAME = "192.168.0.xxx"
MQTT_BROKER_PORT = 1883
DHT_PIN = 4
SECONDS_TO_SLEEP = 4
```

2\) Run `chmod +x mqtt-dht.py` to make the file executable

3\) Copy the service file from this directory to `/etc/systemd/system/mqtt-dht.service`, reload the service configuration with `sudo systemctl deamon-reload`, enable (= autostart) the service with `sudo systemctl enable mqtt-dht` and finally run it with `sudo systemctl start mqtt-dht`

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
    availability_topic: "room/climate/status"
    payload_available: "online"
    payload_not_available: "offline"
    device_class: humidity
```