#include "common.h"
#include "tickerScheduler.h"

#include <Arduino.h>

// Ref: http://www.schatenseite.de/en/2016/04/22/esp8266-witty-cloud-module/
static const byte heartBeatPin = 12; // GREEN_LED (in RGB)

static void heartBeatTick()
{
    static int hbValue = 1;
    static const int hbIncrementScale = 15;
    static int hbIncrement = hbIncrementScale;

    if (getFlag(state.flags, stateFlagsDisableHeartbeat))
    {
        if (hbValue)
        {
            hbValue = 0;
            analogWrite(heartBeatPin, hbValue);
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
    analogWrite(heartBeatPin, hbValue);
}

void initHeartBeat(TickerScheduler &ts)
{
    pinMode(heartBeatPin, OUTPUT);
    ts.sched(heartBeatTick, 100);
}
