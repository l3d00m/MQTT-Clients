1\) Create a file called `config.py` with the following contents:

```python
MOVEMENT_TOPIC = "room/movement"
MQTT_BROKER_HOSTNAME = "192.168.0.xxx"
MQTT_BROKER_PORT = "1883"
PIR_GPIO_PIN = 11
# When it should drop back to false after the last movement
MOVEMENT_TIMEOUT_SECONDS = 120.0
```

2\) Copy the service file from this directory to `/etc/systemd/system/mqtt-pir.service`, reload the service configuration with `sudo systemctl deamon-reload`, enable (= autostart) the service with `sudo systemctl enable mqtt-pir` and finally run it with `sudo systemctl start mqtt-pir`

3\) Add this to your homeassistant configuration:

```yaml
binary_sensor:
  - platform: mqtt
    name: "Movement"
    state_topic: "room/movement"
    payload_on : 1
    payload_off: 0
```