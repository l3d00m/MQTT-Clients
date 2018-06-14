
```yaml
sensor:
  - platform: mqtt
    name: "Temperature"
    state_topic: "room/temperature"
    expire_after: 300
    unit_of_measurement: '°C'
    device_class: temperature
  - platform: mqtt
    name: "Humidity"
    state_topic: "room/humidity"
    unit_of_measurement: '%'
    expire_after: 300
    device_class: humidity
```