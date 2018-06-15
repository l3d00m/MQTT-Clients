
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