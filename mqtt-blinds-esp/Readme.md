Works with the ESP8266

Create a file called `Constants.h` with the following contents:

```cpp
#define wifi_ssid "********"
#define wifi_password "******"
#define hostname "ESP_BLINDS"

#define mqtt_server "192.168.xxx.xxx"

// Where to receive commands
#define cmnd_topic "room/blinds/cmnd"
// Where the current state (as percentage) get's published to
#define stat_topic "room/blinds/stat"
// To manually set the blinds' position
#define pos_topic "room/blinds/pos"
// To override the speed (ie delay) for the next command
#define speed_topic "room/blinds/speed"
// Availibity (via MQTT will)
#define availability_topic "room/blinds/available"

// Pins for the stepper motor
const int motorPin1 = 16;
const int motorPin2 = 5;
const int motorPin3 = 4;
const int motorPin4 = 0;

// Delay between each motor step
const int STEP_DELAY = 3;

```


Home Assistant config:

```yaml
cover:
  - platform: mqtt
    name: "Blinds"
    command_topic: "room/jalousie/cmnd"
    state_topic: "room/jalousie/stat"
    set_position_topic: "room/jalousie/pos"
    availability_topic: "room/jalousie/available"
    qos: 1
    retain: false
    payload_open: "OPEN"
    payload_close: "CLOSE"
    payload_stop: "STOP"
```