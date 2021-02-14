#include "common.h"
#include "tickerScheduler.h"

#include <Arduino.h>

// Ref: http://www.schatenseite.de/en/2016/04/22/esp8266-witty-cloud-module/
static const byte heartBeatGreenPin = 12;
static const byte heartBeatRedPin = 13;

static byte currHeartBeatPin = heartBeatGreenPin;

static void setHbLeds(const bool setRedLed)
{
    digitalWrite(heartBeatGreenPin, setRedLed ? LOW : HIGH);
    digitalWrite(heartBeatRedPin, setRedLed ? HIGH : LOW);
}

void updateSensorDisableLed()
{
    const bool isDisabled = getFlag(state.flags, stateFlagsDisableSensor);
    currHeartBeatPin = isDisabled ? heartBeatRedPin: heartBeatGreenPin;
    setHbLeds(isDisabled);
}

static void heartBeatCrazyLedTick()
{
    static bool setRedLed;
    if (!getFlag(state.flags, stateFlagsCrazyLed)) {
        return;
    }
    setHbLeds(setRedLed);
    setRedLed = !setRedLed;
}

static void heartBeatTick()
{
    static int hbValue = 1;
    static const int hbIncrementScale = 15;
    static int hbIncrement = hbIncrementScale;

    if (getFlag(state.flags, stateFlagsCrazyLed)) {
        return;
    }

    if (getFlag(state.flags, stateFlagsDisableHeartbeat))
    {
        if (hbValue)
        {
            hbValue = 0;
            setHbLeds(getFlag(state.flags, stateFlagsDisableSensor));
        }
        return;
    }

    hbValue += hbIncrement;

    if (hbValue >= 256)
    {
        hbValue = 255;
        hbIncrement = hbIncrementScale * -1;
    }
    else if (hbValue <= -1)
    {
        hbValue = 0;
        hbIncrement = hbIncrementScale;
    }
    analogWrite(currHeartBeatPin, hbValue);
}

void initHeartBeat(TickerScheduler &ts)
{
    pinMode(heartBeatGreenPin, OUTPUT);
    pinMode(heartBeatRedPin, OUTPUT);
    ts.sched(heartBeatTick, 100);
    ts.sched(heartBeatCrazyLedTick, 250);
}
