# IoT MQTT Gizmo

Reference IoT project using [platformio.org][pio] with OTA on ESP8266 with SHT31 (or DHT22) for monitoring via MQTT

### Goals

- Use [Witty Cloud][witty] Module + [SHT31-D][sht31] (or [DHT22][dht22]) to monitor ambient light, temperature, and humidity
- Keep it all as cheap as possible
- Leverage [Arduino Over The Air][ota] library with [platformio.org][pio] for easy code updates
- Make code simple, so it can be easily changed to support different hardware

### Hardware parts

Note: these are the parts I used, but I really hope you can easily leverage this repo to use whatever makes sense to you.

- [Witty Cloud][witty] Module. You can buy one for cheap at many places, such as [DealExtreme](https://www.dx.com/p/esp8266-serial-esp-12f-wi-fi-witty-cloud-development-board-black-2068729.html) or [Amazon](https://amazon.com/dp/B07V2DZCYK/ref=cm_sw_em_r_mt_dp_kqtxFb26W1EPY).
- **Option 1**: [SHT31-D Temperature & Humidity Sensor Breakout][sht31]. Although I'd recommend getting the [one from Adafruit](https://www.adafruit.com/product/2857), this can be found at places like [Aliexpress](https://www.aliexpress.com/price/sht31_price.html) as well.
- **Option 2**: [DHT22 thermometer][dht22] with jumper cables. Also something you can easily get in places such as [DealExtreme](https://www.dx.com/p/dht22-2302-digital-temperature-and-humidity-sensor-module-2023234.html) or [Amazon](https://amazon.com/dp/B073F472JL/ref=cm_sw_em_r_mt_dp_pttxFb48ZJCY9), or [Adafruit](https://www.adafruit.com/product/393).
- **Option 3**: Try [different sensors](https://www.adafruit.com/?q=temperature+sensors&sort=BestMatch) and make a PR for this repo!
- Cable with micro USB port (a.k.a Micro Type B connector)
- [Female jumper cables](https://www.amazon.com/GenBasic-Piece-Female-Jumper-Wires/dp/B01L5ULRUA/ref=sr_1_5?dchild=1&keywords=electrical+female+jumper+wires&qid=1615141532&sr=8-5)
- Power adapter or battery holder to power the Witty Cloud

There is no soldering needed at all for this project. The hookup of the temperature sensor uses the VCC, Ground, and I2C (data/clock) pins.
For the Witty, these are GPIO4(SDA) and GPIO5(SCL). See [this link](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/) and pictures below for reference.

![witty with case](https://live.staticflickr.com/65535/51011525173_d20f73a1c5_4k.jpg)

### **NOTE for DHT22 users**  :warning:
If you choose to use the [DHT22 thermometer][dht22], be sure to use the [dht22 branch](https://github.com/flavio-fernandes/iotMqttGizmo/tree/dht22) of this repo. The main branch is currently tailored for the [STH31][sht31] sensor.


### Pre-requisites

- [platformio.org][pio] environment (see [platformio-ide](https://platformio.org/platformio-ide))
- WIFI and an MQTT server

If you do not want to set an MQTT server up, consider [using a pubic](https://github.com/mqtt/mqtt.github.io/wiki/public_brokers) one, like [Adafruit.IO](https://io.adafruit.com/)

### Configuration

After cloning this repo, there are 2 files that you must modify before being able to
compile and upload the code:

#### (1) **platformio.ini**

Rename _[platformio.ini.sample](https://github.com/flavio-fernandes/iotMqttGizmo/blob/93d035c7597090cd8ba87866c337a9f7e87f9ffa/platformio.ini.sample)_ to _platformio.ini_ and change the settings to match the device you are using.

#### (2) **include/netConfig.h**

Rename _[netConfig.h.sample](https://github.com/flavio-fernandes/iotMqttGizmo/blob/93d035c7597090cd8ba87866c337a9f7e87f9ffa/include/netConfig.h.sample)_ to _netConfig.h_ and make the necessary changes to access the WIFI and MQTT server.
This file also configures the MQTT prefix for the topic your device will use (Look for [DEV_PREFIX](https://github.com/flavio-fernandes/iotMqttGizmo/blob/93d035c7597090cd8ba87866c337a9f7e87f9ffa/include/netConfig.h.sample#L15)).

### Interacting via MQTT

Once operational, the following MQTT topics will be [periodically](https://github.com/flavio-fernandes/iotMqttGizmo/blob/93d035c7597090cd8ba87866c337a9f7e87f9ffa/src/net.cpp#L438) published by the device:

- /DEV_PREFIX/light        (brightness value, [from 0 to 1023](https://randomnerdtutorials.com/esp8266-adc-reading-analog-values-with-nodemcu/))
- /DEV_PREFIX/temperature  (in Fahrenheit *)
- /DEV_PREFIX/humidity     (percentage)

These may also be published if a [change is detected](https://github.com/flavio-fernandes/iotMqttGizmo/blob/93d035c7597090cd8ba87866c337a9f7e87f9ffa/src/net.cpp#L392-L404) regardless of the publishing interval cycle.
\* If you rather have the temperature in Celcius, comment out the [convert](https://github.com/flavio-fernandes/iotMqttGizmo/blob/93d035c7597090cd8ba87866c337a9f7e87f9ffa/src/temperature.cpp#L39) function.


There are 3 topics you can publish to control the device. Actually, [there are 2 more](https://github.com/flavio-fernandes/iotMqttGizmo/blob/93d035c7597090cd8ba87866c337a9f7e87f9ffa/src/net.cpp#L17-L20), but they are just for debug. :sweat_smile:

- /DEV_PREFIX/hb : controls the heartbeat led
- /DEV_PREFIX/sensor : controls the publishing of light, humidity and temperature values
- /DEV_PREFIX/heater : controls the heater feature of the [SHT31-D][sht31]. For more info on that, [check page 8 from this link](https://www.glynshop.com/erp/owweb/Daten/Datenblaetter/Sensirion/SHT3x_LSS_V0.93.pdf).

When set with values '**on**' or '**off**' or '**toggle**' you will enable/disable/flip the state. By default, `hb` and `sensor` are '**on**' when the device boots (`heater` will be '**off**' by default).

### Example MQTT session

Any MQTT client will work. The one I use quite often and really like is called Mosquitto.
More info at [mosquitto.org/download/](https://mosquitto.org/download/).
Here is a cheat sheet set of commands for easily getting it in your computer:

```bash
$ # If you are using Debian / Ubuntu / Raspberry-Pi:
$ sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa
$ sudo apt-get update
$ sudo apt-get install -y mosquitto-clients

$ # If you are using a mac:
$ brew install mosquitto
```

Here is an example using mosquitto client to interact with the device.

```bash
$ # shell window 1
$ MQTT=192.168.123.123 ; DEV_PREFIX='test'
$ mosquitto_sub -F '@Y-@m-@dT@H:@M:@S@z : %q : %t : %p' -h $MQTT -t "/${DEV_PREFIX}/#"
2020-09-11T19:11:00-0400 : 0 : /test/light : 20
2020-09-11T19:11:00-0400 : 0 : /test/temperature : 73
2020-09-11T19:11:00-0400 : 0 : /test/humidity : 44
```

```bash
$ # shell window 2
$ MQTT=192.168.123.123 ; DEV_PREFIX='test'
$ mosquitto_pub -h $MQTT -t "/${DEV_PREFIX}/hb" -r -m on
$ mosquitto_pub -h $MQTT -t "/${DEV_PREFIX}/heater" -m off
$ mosquitto_pub -h $MQTT -t "/${DEV_PREFIX}/sensor" -m off
$ mosquitto_pub -h $MQTT -t "/${DEV_PREFIX}/sensor" -m toggle

$ # backdoor for disabling both hb and sensor. Don't do it! ;)
$ mosquitto_pub -h $MQTT -t "/${DEV_PREFIX}/flags" -m 3
$ # dog timer reset. Not used unless you mess with NEED_PERIODIC_PINGS
$ mosquitto_pub -h $MQTT -t "/${DEV_PREFIX}/ping" -m periodic
$ # causes it to publish on all topics
$ mosquitto_pub -h $MQTT -t "/${DEV_PREFIX}/ping" -r -n
```

### 3d Case

Use the following link to print a case for this project:

- [ESP8266 witty case -- thing 1434261](https://www.thingiverse.com/thing:1434261)


## Pictures

More pictures of this project are [available here](https://flic.kr/s/aHsmUG2fNW).


![witty with SHT31-D](https://live.staticflickr.com/65535/51012238811_520b679392_4k.jpg)
![witty with SHT31-D angle 2](https://live.staticflickr.com/65535/51012238851_1367dfe5f5_4k.jpg)


[pio]: https://platformio.org/ "platformio.org"
[witty]: http://www.schatenseite.de/en/2016/04/22/esp8266-witty-cloud-module/ "Witty Cloud Module"
[dht22]: https://learn.adafruit.com/dht/overview "Temperature and Humidity sensor"
[sht31]: https://learn.adafruit.com/adafruit-sht31-d-temperature-and-humidity-sensor-breakout "Adafruit SHT31-D Temperature & Humidity Sensor Breakout"
[ota]: http://arduino.esp8266.com/Arduino/versions/2.0.0/doc/ota_updates/ota_updates.html "Arduino Over The Air Library"
