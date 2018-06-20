# Collection of various MQTT clients

Optimized for usage with [Home Assistant](https://www.home-assistant.io/hassio/)

## DHT module

Converts a DHT sensor into an mqtt client. It has inbuilt validity check for values that are too high. It pushes humidity and temperature to their respective mqtt topics. It pushes only on change so the Home assistant database does not get to polluted. It also has an availability topic.

## PIR module

Support for a cheap PIR movement sensor. It has a custom turn-off delay. If the sensor has an inbuilt hardware delay, it should be disabled. The advantage is that the delay can be exactly specified as a constant.

## Screen module  

TBD

## ESP blinds module

TBD