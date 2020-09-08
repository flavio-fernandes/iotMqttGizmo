#include "common.h"
#include <Esp.h>

// Ref: https://github.com/arduino/ArduinoCore-avr/issues/251
#ifndef bitSet64
#define bitSet64(value, bit) (value |= 1ULL << bit)
#endif

#ifndef bitClear64
#define bitClear64(value, bit) (value &= ~(1ULL << bit))
#endif

static void refreshFlags(const uint64_t & /*currFlags*/)
{
  updateTemperatureSensorCachedValue();
  updateSensorDisableLed();
}

void setFlags(uint64_t &currFlags, uint64_t flags)
{
  if (currFlags == flags)
    return;
  currFlags = flags;
  refreshFlags(currFlags);
}

bool getFlag(const uint64_t &currFlags, int flagBit)
{
  if (flagBit < 0 || flagBit > 63)
    return false;
  return bitRead(currFlags, flagBit) != 0;
}

bool setFlag(uint64_t &currFlags, int flagBit)
{
  const uint64_t origFlags = currFlags;
  if (flagBit < 0 || flagBit > 63)
    return false;
  bitSet64(currFlags, flagBit);
  if (origFlags != currFlags)
  {
    refreshFlags(currFlags);
    return true;
  }
  return false;
}

bool clearFlag(uint64_t &currFlags, int flagBit)
{
  const uint64_t origFlags = currFlags;
  if (flagBit < 0 || flagBit > 63)
    return false;
  bitClear64(currFlags, flagBit);
  if (origFlags != currFlags)
  {
    refreshFlags(currFlags);
    return true;
  }
  return false;
}

bool flipFlag(uint64_t &currFlags, int flagBit)
{
  if (flagBit < 0 || flagBit > 63)
    return false;
  const bool currBit = bitRead(currFlags, flagBit) == 1;
  return currBit ? clearFlag(currFlags, flagBit) : setFlag(currFlags, flagBit);
}

void parseOnOffToggle(const char *subName, const char *message,
                      OnOffToggle onPtr, OnOffToggle offPtr, OnOffToggle togglePtr)
{
  if (onPtr != nullptr && strncmp(message, "on", 2) == 0)
  {
    (*onPtr)();
  }
  else if (offPtr != nullptr && strncmp(message, "off", 3) == 0)
  {
    (*offPtr)();
  }
  else if (onPtr != nullptr && strncmp(message, "1", 1) == 0)
  {
    (*onPtr)();
  }
  else if (offPtr != nullptr && strncmp(message, "0", 1) == 0)
  {
    (*offPtr)();
  }
  else if (togglePtr != nullptr && strncmp(message, "toggle", 6) == 0)
  {
    (*togglePtr)();
  }

#ifdef DEBUG
  Serial.printf("got msg on %s : %s\n", subName, message);
#endif
}

void gameOver(const char *const msg)
{
#ifdef DEBUG
  Serial.printf("PANIC: gameOver: %s\n", msg);
#endif
  ESP.restart();
}
