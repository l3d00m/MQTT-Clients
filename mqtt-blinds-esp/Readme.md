Works with the ESP8266

Create a file called `Constants.h` with the following contents:

```cpp
#define wifi_ssid "********"
#define wifi_password "******"
#define hostname "ESP_BLINDS"
#define ota_password "XXXXXXX"

#define mqtt_server "192.168.xxx.xxx"

// Where to receive commands
#define cmnd_topic "room/blinds/cmnd"
// Where the current state (as percentage) get's published to
#define stat_topic "room/blinds/stat"
// To manually set the blinds' position
#define pos_topic "room/blinds/pos"
// To override the speed (ie rpm) for the next command
#define next_speed_topic "room/blinds/next_speed"
// Availibity (via MQTT will)
#define availability_topic "room/blinds/available"
// To configure the speed
#define speed_config_topic "room/blinds/config/speed"
// To configure the total steps (e.g. how far the blinds will go)
#define steps_config_topic "room/blinds/config/steps"

// Pins for the stepper motor
const int motorPin1 = 16;
const int motorPin2 = 5;
const int motorPin3 = 4;
const int motorPin4 = 0;

// Optional, Delay between each motor step, can be overriden by mqtt config
const int INIT_RPM = 13;
// How long to leave the motor on after it stopped (to avoid the blinds falling down because of sudden stop)
const int HOLD_DELAY_MS = 1000;

```


Home Assistant config:

```yaml
cover:
  - platform: mqtt
    name: "Blinds"
    command_topic: "room/blinds/cmnd"
    state_topic: "room/blinds/stat"
    set_position_topic: "room/blinds/pos"
    availability_topic: "room/blinds/available"
    qos: 1
    retain: false
    payload_open: "OPEN"
    payload_close: "CLOSE"
    payload_stop: "STOP"
```