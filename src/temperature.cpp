#include "common.h"
#include "tickerScheduler.h"

// http://www.schatenseite.de/en/2016/04/22/esp8266-witty-cloud-module/
// https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHT_Unified_Sensor/DHT_Unified_Sensor.ino

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 5 // Digital pin connected to the DHT sensor.
// ESP8266 – Witty Cloud Module note: use pins:  16, 14, 5

// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

static void debugPrintTemperatureSensor()
{
#ifdef DEBUG
    Serial.print(F("Temperature: "));
    Serial.print(state.temperature);
    Serial.print(F("°F"));
    Serial.print(F("  --  Humidity: "));
    Serial.print(state.humidity);
    Serial.println(F("%"));
#endif
}

static float convertCtoF(float c) { return c * 1.8 + 32; }

void updateTemperatureSensorCachedValue()
{
    if (getFlag(state.flags, stateFlagsDisableSensor))
        return;

    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature))
    {
#ifdef DEBUG
        Serial.println(F("Error reading temperature!"));
#endif
        return;
    }
    state.temperature = convertCtoF(event.temperature);

    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity))
    {
#ifdef DEBUG
        Serial.println(F("Error reading humidity!"));
#endif
        return;
    }
    state.humidity = event.relative_humidity;
    debugPrintTemperatureSensor();
}

void initTemperatureSensor(TickerScheduler &ts)
{
    // Initialize device.
    dht.begin();

    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
#ifdef DEBUG
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(convertCtoF(sensor.max_value));
    Serial.println(F("°F"));
    Serial.print(F("Min Value:   "));
    Serial.print(convertCtoF(sensor.min_value));
    Serial.println(F("°F"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("°C"));
    Serial.println(F("------------------------------------"));
#endif

    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
#ifdef DEBUG
    Serial.println(F("Humidity Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(sensor.max_value);
    Serial.println(F("%"));
    Serial.print(F("Min Value:   "));
    Serial.print(sensor.min_value);
    Serial.println(F("%"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("%"));
    Serial.println(F("------------------------------------"));
#endif

    const uint32_t delayMS = sensor.min_delay / 1000;
    delay(delayMS);
    updateTemperatureSensorCachedValue();

    const uint32_t oneSec = 1000;
    ts.sched(updateTemperatureSensorCachedValue, oneSec * 45 + delayMS);
    //ts.sched(debugPrintTemperatureSensor, oneSec * 90);
}
