
WiFi PIR Sensor/Luxmeter for ESP8266
====================================

This is simple PIR sensor and luxmeter controlled over MQTT.
It uses the [Homie MQTT convention](https://github.com/marvinroger/homie)
and is based on [Homie implementation for ESP8266](https://github.com/marvinroger/homie-esp8266) by Marvin Roger.

Wiring
------

photocell goes to A0
and the PIR sensor to D5

![wiring](doc/pir_sensor_bb.svg)

Instalation
-----------

Import into [platformio](http://platformio.org/), then run build target "PlatformIO: Upload SPIFFS Image".

Adjust the photocell type in the code in [src/main.cpp](src/main.cpp#L5)
to match whatever you have.
(You may need to follow the instructions for the [Arduino Light Dependent Resistor Library](https://github.com/QuentinCG/Arduino-Light-Dependent-Resistor-Library/)
if your photocell is not supported by the library directly)

Then you could compile and upload the sketch to your board as usual.

Once you upload the sketch, you have to configure the node as described in
[Homie Getting Started Tutorial](https://homie-esp8266.readme.io/docs/getting-started)
(this is where you device gets device id assigned)

Interface
---------

- The device will report the status from PIR sensor to MQTT topic `homie/<deviceid>/pir/motion`

```bash
$ mosquitto_sub -h 192.168.1.50 -v -t "homie/a020a61590a1/pir/motion"
homie/a020a61590a1/pir/motion NO
homie/a020a61590a1/pir/motion YES
homie/a020a61590a1/pir/motion NO
```

- The device will report light intensity (in [lx](https://en.wikipedia.org/wiki/Lux))
from photocell to MQTT topic `homie/<deviceid>/photocell/lux`

```bash
$ mosquitto_sub -h 192.168.1.50 -v -t "homie/a020a61590a1/photocell/lux"
homie/a020a61590a1/photocell/lux 118.61
```

to adjust the interval how often is the light intensity reported publish
the new value to "homie/<deviceid>/photocell/interval/set" (in seconds)

```bash
$ mosquitto_pub -h 192.168.1.50 -t "homie/a020a61590a1/photocell/interval/set" -m "5"
```

- similarly you could control the built-in LED by publishing to following topics

```
homie/<deviceid>/led/brightness/set <0..1023>
homie/<deviceid>/led/status/set  <ON|OFF>
```
