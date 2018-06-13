

```yaml
binary_sensor:
  - platform: mqtt
    name: "Movement"
    state_topic: "room/bewegung/stat"
    payload_on : 1
    payload_off: 0

```