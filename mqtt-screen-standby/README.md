1\) Create a file called `config.py` with the following contents:

```python
STATE_TOPIC = 'room/monitor/stat'
COMMAND_TOPIC = 'room/monitor/cmnd'
MQTT_BROKER_PORT = 1883
MQTT_BROKER_HOST = "192.168.xxx.xxx"
AVAILABILITY_TOPIC = "room/monitor/available"
```

2\) To let the script run as a service on debian:
 1. Symlink the service file from this directory to `/etc/systemd/system/mqtt-screen-standby.service`, e.g. `sudo ln -sf /home/pi/MQTT-Clients/mqtt-screen-standby/mqtt.service /etc/systemd/system/mqtt-screen-standby.service`
 2. Reload the service configuration with `sudo systemctl deamon-reload`
 3. Enable (= autostart) the service with `sudo systemctl enable mqtt-screen-standby` 
 4. Run it with `sudo systemctl start mqtt-screen-standby`

3\) Add this to your homeassistant configuration:

```yaml
switch:
  - platform: mqtt
    name: "Screen"
    command_topic: "room/monitor/cmnd"
    state_topic: "room/monitor/stat"
    icon: mdi:monitor
    availability_topic: "room/monitor/available"
    payload_available: "online"
    payload_not_available: "offline"
```
