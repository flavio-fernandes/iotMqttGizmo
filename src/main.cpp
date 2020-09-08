#include "common.h"
#include "tickerScheduler.h"
#include <cstring>
#include <Arduino.h>

TickerScheduler ts;
State state;

// Ref: http://www.schatenseite.de/en/2016/04/22/esp8266-witty-cloud-module/
static const byte sensorDisabled_Led = 15; // RED_LED
void updateSensorDisableLed()
{
    digitalWrite(sensorDisabled_Led, getFlag(state.flags, stateFlagsDisableSensor) ? HIGH : LOW);
}

static void initGlobals()
{
    memset(&state, 0, sizeof(state));
}

void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
#endif

    // stage 1
    initGlobals();

    // stage 2
    initLightSensor(ts);
    initTemperatureSensor(ts);
    initHeartBeat(ts);
    initButton(ts);
    initMyMqtt(ts);

    pinMode(sensorDisabled_Led, OUTPUT);
    updateSensorDisableLed();

#ifdef DEBUG
    Serial.println("Init finished");
#endif
}

void loop()
{
    ts.update();
    myMqttLoop();
}

void setDisableHb() { setFlag(state.flags, stateFlagsDisableHeartbeat); }
void clearDisableHb() { clearFlag(state.flags, stateFlagsDisableHeartbeat); }
void toggleDisableHb() { flipFlag(state.flags, stateFlagsDisableHeartbeat); }
void setDisableSensor() { setFlag(state.flags, stateFlagsDisableSensor); }
void clearDisableSensor() { clearFlag(state.flags, stateFlagsDisableSensor); }
void toggleDisableSensor() { flipFlag(state.flags, stateFlagsDisableSensor); }
