#include "common.h"
#include "tickerScheduler.h"
#include <cstring>
#include <Arduino.h>

TickerScheduler ts;
State state;

// Ref: http://www.schatenseite.de/en/2016/04/22/esp8266-witty-cloud-module/
static const byte unused_LDR_Pin = A0;
static const byte unused_button_Pin = 4;
static const byte unused_button_Led = 13;
static const byte unused_sensorDisabled_Led = 15;

static void initUnusedPins()
{
    pinMode(unused_LDR_Pin, INPUT);
    pinMode(unused_button_Pin, INPUT);
    pinMode(unused_button_Led, OUTPUT);
    pinMode(unused_sensorDisabled_Led, OUTPUT);
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
    initUnusedPins();

    // stage 2
    initTemperatureSensor(ts);
    initHeartBeat(ts);
    initMyMqtt(ts);

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
void setCrazyLed() { setFlag(state.flags, stateFlagsCrazyLed); }
void clearCrazyLed() { clearFlag(state.flags, stateFlagsCrazyLed); }
void toggleCrazyLed() { flipFlag(state.flags, stateFlagsCrazyLed); }
