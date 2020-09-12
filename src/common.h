#ifndef _COMMON_H

#define _COMMON_H

#include <inttypes.h>

// #define DEBUG 1
// #define NEED_PERIODIC_PINGS 1

class TickerScheduler;

// FWDs decls... utils.cpp
void setFlags(uint64_t &currFlags, uint64_t flags);
bool getFlag(const uint64_t &currFlags, int flagBit);
bool setFlag(uint64_t &currFlags, int flagBit);
bool clearFlag(uint64_t &currFlags, int flagBit);
bool flipFlag(uint64_t &currFlags, int flagBit);
typedef void (*OnOffToggle)();
void parseOnOffToggle(const char *subName, const char *message,
                      OnOffToggle onPtr = nullptr, OnOffToggle offPtr = nullptr,
                      OnOffToggle togglePtr = nullptr);
void gameOver(const char *const msg);

// FWDs decls... main.cpp
void updateSensorDisableLed();
void setDisableHb();
void clearDisableHb();
void toggleDisableHb();
void setDisableSensor();
void clearDisableSensor();
void toggleDisableSensor();

// FWDs decls... heartBeat.cpp, ...
void initHeartBeat(TickerScheduler &ts);
void initLightSensor(TickerScheduler &ts);
void initButton(TickerScheduler &ts);
void initTemperatureSensor(TickerScheduler &ts);
void updateTemperatureSensorCachedValue();

// FWDs decls... net.cpp
void initMyMqtt(TickerScheduler &ts);
void myMqttLoop();
void sendOperState();
void sendLightSensor();
void sendTemperatureSensor();

typedef enum
{
    stateFlagsDisableHeartbeat,
    stateFlagsDisableSensor,
} StateFlags;

typedef struct
{
    uint64_t flags;                       // StateFlags
    uint32_t minutes_uptime;
    uint32_t minutes_since_periodic_ping; // dog

    float temperature;
    float humidity;
    int lightSensor;
} State;

extern State state;

#endif // _COMMON_H
