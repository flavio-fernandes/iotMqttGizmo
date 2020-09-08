#include "common.h"
#include "tickerScheduler.h"

#include <Arduino.h>

// Ref: http://www.schatenseite.de/en/2016/04/22/esp8266-witty-cloud-module/
//      http://playground.arduino.cc/Learning/PhotoResistor

//             PhotoR     10K
//   +5    o---/\/\/--.--/\/\/---o GND
//                    |
//  Pin A0 o-----------

static const uint8_t LDR_Pin = A0;

static void updateLightSensorCachedValue()
{

    if (getFlag(state.flags, stateFlagsDisableSensor))
    {
        if (state.lightSensor)
        {
            state.lightSensor = 0;
        }
        return;
    }

    state.lightSensor = analogRead(LDR_Pin);
}

static void debugPrintLightSensor()
{
    if (getFlag(state.flags, stateFlagsDisableSensor))
        return;

#ifdef DEBUG
    Serial.printf("Light sensor: %u\n", state.lightSensor);
#endif // #ifdef DEBUG
}

void initLightSensor(TickerScheduler &ts)
{
    pinMode(LDR_Pin, INPUT);

    // initial
    updateLightSensorCachedValue();

    const uint32_t oneSec = 1000;
    ts.sched(updateLightSensorCachedValue, oneSec * 3);
    ts.sched(debugPrintLightSensor, oneSec * 60);
}
