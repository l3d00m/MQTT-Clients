Works with the ESP8266

Create a file called `Constants.h` with the following contents:

```cpp
// WiFi settings
#define wifi_ssid "********"
#define wifi_password "******"
#define ota_password "XXXXXXX"

// Hostname / clientname for wifi and mqtt
#define clientname "ESP_BLINDS"

// MQTT settings
#define mqtt_port 1883
#define mqtt_server "192.168.xxx.xxx"
#define topic_prefix "room1/blinds/" // end with slash !

// Pins for the stepper motor
#define motorPin1 16
#define motorPin2 5
#define motorPin3 4
#define motorPin4 0
```


Home Assistant config:

```yaml
cover:
  - platform: mqtt
    name: "Blinds in Room1"
    command_topic: "room1/blinds/cmnd"
    position_topic: "room1/blinds/stat"
    set_position_topic: "room1/blinds/pos"
    availability_topic: "room1/blinds/available"
    qos: 1
```
