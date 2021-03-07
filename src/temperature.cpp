#include "common.h"
#include "tickerScheduler.h"

// http://www.schatenseite.de/en/2016/04/22/esp8266-witty-cloud-module/
// https://github.com/adafruit/Adafruit_SHT31
// https://www.adafruit.com/product/2857

#include <Arduino.h>
#include "Adafruit_SHT31.h"

static Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Use SCL and SDA pins. In ESP8266 – Witty, those are labeled on silk screen
// GPIO5 (SCL) and GPIO4 (SDA)
// Ref: https://images.app.goo.gl/xV7ttxv5BHcLP3C3A

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
  const bool heaterEnabled = sht31.isHeaterEnabled();

  if (getFlag(state.flags, stateFlagsDisableSensor))
  {
    if (heaterEnabled)
    {
      sht31.heater(false);  // Disabled when sensor is disabled
    }
    return;
  }

  const float t = sht31.readTemperature();
  const float h = sht31.readHumidity();
  const bool heaterEnabledWanted = getFlag(state.flags, stateFlagsEnableHeater);

  if (heaterEnabled != heaterEnabledWanted)
  {
#ifdef DEBUG
    Serial.printf("Setting SHT31 heater: %s\n", heaterEnabledWanted ? "on" : "off");
#endif
    sht31.heater(heaterEnabledWanted);
  }

  if (!isnan(t))
  { // check if 'is not a number'
    state.temperature = convertCtoF(t);
  }
#ifdef DEBUG
  else
  {
    Serial.println("Failed to read temperature");
  }
#endif

  if (!isnan(h))
  { // check if 'is not a number'
    state.humidity = h;
  }
#ifdef DEBUG
  else
  {
    Serial.println("Failed to read humidity");
  }
#endif
  debugPrintTemperatureSensor();
}

void initTemperatureSensor(TickerScheduler &ts)
{
  // Initialize device.
  if (!sht31.begin(0x44))
  { // Set to 0x45 for alternate i2c addr
    const char *const msg = "Couldn't find SHT31";
#ifdef DEBUG
    Serial.println(msg);
#endif
    delay(10000);
    gameOver(msg);
  }

#ifdef DEBUG
  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
#endif

  updateTemperatureSensorCachedValue();

  const uint32_t oneSec = 1000;
  ts.sched(updateTemperatureSensorCachedValue, oneSec * 45);
  //ts.sched(debugPrintTemperatureSensor, oneSec * 90);
}
