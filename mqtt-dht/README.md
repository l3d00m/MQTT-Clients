1\) Create a file called `config.py` with the following contents:

```python
TEMPERATURE_TOPIC = "room/climate/temperature"
HUMIDITY_TOPIC = "room/climate/humidity"
AVAILABILITY_TOPIC = "room/climate/status"
MQTT_BROKER_HOSTNAME = "192.168.0.xxx"
MQTT_BROKER_PORT = 1883
SECONDS_TO_SLEEP = 2
HASS_NAME_TEMPERATURE = "Room Temperature"
HASS_NAME_HUMIDITY = "Room Humidity"
```

DHT PIN is always GPIO4.

2\) Install the required dependencies:
 1. Install apt dependencies: `sudo apt install python3 python3-pip libgpiod2`
 1. Execute the following command in this folder to install the python dependencies: `sudo pip3 install -r requirements.txt`

3\) Make the script run as a service on debian:
 1. Symlink the service file from this directory to `/etc/systemd/system/mqtt-dht.service`, e.g. `sudo ln -sf /home/pi/MQTT-Clients/mqtt-dht/mqtt-dht.service /etc/systemd/system/mqtt-dht.service`
 2. Reload the service configuration with `sudo systemctl daemon-reload`
 3. Enable (= autostart) the service with `sudo systemctl enable mqtt-dht` 
 4. Run it with `sudo systemctl start mqtt-dht`


5\)
Add an optional filter sensor in homeassistant to correct invalid readings, smoothen the result for a visualization or for storing less values. My setup currently is:
````yaml
  - platform: filter
    name: "Filtered temperature"
    entity_id: sensor.temperature
    filters:
      - filter: range # filter everything below 5 degree because it's an inside sensor
        lower_bound: 5
      - filter: outlier # add an outlier filter which is useful for incorrect reading which happends when used without pullup resisitor
        window_size: 0.6
      - filter: time_simple_moving_average # add a moving average filter to smoothen the result and in the next step
        window_size: 00:10
      - filter: time_throttle # only one new value every x minutes for keeping the db small. Works best with the moving average from above
        window_size: 00:08

````
