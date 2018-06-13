
```yaml
sensor:
  - platform: mqtt
    name: "Temperature"
    state_topic: "room/temperatur"
    value_template: "{{ value_json.temperatur }}"
    expire_after: 300
    unit_of_measurement: '  C'
    device_class: temperature
  - platform: mqtt
    name: "Humidity"
    state_topic: "room/temperatur"
    unit_of_measurement: '%'
    expire_after: 300
    value_template: "{{ value_json.humidity }}"
    device_class: humidity
```