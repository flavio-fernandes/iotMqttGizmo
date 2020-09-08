#include "common.h"
#include "tickerScheduler.h"

#include <Arduino.h>

// Ref: http://www.schatenseite.de/en/2016/04/22/esp8266-witty-cloud-module/
static const uint8_t button_Pin = 4; // pin connected to Witty Cloud Module button
static const byte button_Led = 13;   // BLUE_LED (in RGB, not led_builtin)

static void checkButton()
{
    static int lastButtonState = HIGH;

    const int buttonState = digitalRead(button_Pin);
    if (buttonState == lastButtonState)
        return;

    lastButtonState = buttonState;
    digitalWrite(button_Led, !buttonState);

#ifdef DEBUG
    Serial.printf("Button %s\n", buttonState == HIGH ? "released" : "pressed");
#endif // #ifdef DEBUG

    if (buttonState == HIGH)
    { // when button is released...
        updateTemperatureSensorCachedValue();
        sendOperState();
        sendLightSensor();
        sendTemperatureSensor();
    }
}

void initButton(TickerScheduler &ts)
{
    pinMode(button_Pin, INPUT);
    pinMode(button_Led, OUTPUT);

    digitalWrite(button_Led, LOW);
    ts.sched(checkButton, 333);
}
