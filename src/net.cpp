#include "common.h"
#include "netConfig.h"
#include "tickerScheduler.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define PIN_MQTT_LED LED_BUILTIN
#define PIN_MQTT_LED_OFF HIGH
#define PIN_MQTT_LED_ON LOW

#define MQTT_SUB_PING "ping"
#define MQTT_SUB_HB "hb"
#define MQTT_SUB_HEATER "heater"
#define MQTT_SUB_SENSOR "sensor"
#define MQTT_XUB_FLAGS "flags"

#define MQTT_PUB_LIGHT "light"
#define MQTT_PUB_TEMPERATURE "temperature"
#define MQTT_PUB_HUMIDITY "humidity"

// FWDs
static bool isMqttConnected();
static bool checkWifiConnected();
static bool checkMqttConnected();

static void mqtt1SecTick();
static void mqtt1MinTick();
static void mqtt10MinTick();

typedef struct MqttConfig_t
{
    Adafruit_MQTT_Subscribe *service_sub_ping;
    Adafruit_MQTT_Subscribe *service_sub_heartbeat;
    Adafruit_MQTT_Subscribe *service_sub_heater;
    Adafruit_MQTT_Subscribe *service_sub_sensor;
    Adafruit_MQTT_Subscribe *service_sub_flags;

    Adafruit_MQTT_Publish *service_pub_flags;
    Adafruit_MQTT_Publish *service_pub_light;
    Adafruit_MQTT_Publish *service_pub_temperature;
    Adafruit_MQTT_Publish *service_pub_humidity;

    Adafruit_MQTT_Client *mqttPtr;
    const char *topicPing;
    const char *topicHeartbeat;
    const char *topicHeater;
    const char *topicSensor;
    const char *topicFlags;
    const char *topicLight;
    const char *topicTemperature;
    const char *topicHumidity;
} MqttConfig;

static struct MqttConfig_t mqttConfig = {0};

static const unsigned int defaultMqttReconnect = 30;
typedef struct
{
    bool lastMqttConnected;      // state of mqtt
    unsigned int reconnectTicks; // do not mqtt connect while this is > 0
} MqttState;

static MqttState mqttState;

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

static void initOTA()
{
#ifdef OTA_PORT
    // Port defaults to 8266
    ArduinoOTA.setPort(OTA_PORT);
#endif // #ifdef OTA_PORT

#ifdef OTA_HOSTNAME
    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(OTA_HOSTNAME);
#endif // #ifdef OTA_HOSTNAME

#ifdef OTA_PASS
    // No authentication by default
    ArduinoOTA.setPassword(OTA_PASS);
#elifdef OTA_MD5
    // Password can be set with it's md5 value as well
    ArduinoOTA.setPasswordHash(OTA_MD5);
#endif // #ifdef OTA_PASS

#ifdef DEBUG
    ArduinoOTA.onStart([]() { Serial.println("Start updating"); });
    ArduinoOTA.onEnd([]() { Serial.println("Ended updating"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
        {
            Serial.println("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            Serial.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            Serial.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            Serial.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            Serial.println("End Failed");
        }
    });
#endif
}

void initMyMqtt(TickerScheduler &ts)
{
    pinMode(PIN_MQTT_LED, OUTPUT);
    digitalWrite(PIN_MQTT_LED, PIN_MQTT_LED_OFF);

#ifdef DEBUG
    Serial.printf("\nConnecting to %s\n", WLAN_SSID);
#endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    initOTA();

    memset(&mqttState, 0, sizeof(mqttState));
    mqttState.lastMqttConnected = false;
    // Wait before attempting first mqtt connect
    mqttState.reconnectTicks = 6;

    mqttConfig.mqttPtr = new Adafruit_MQTT_Client(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
    String tmp;

    tmp = DEV_PREFIX MQTT_SUB_PING;
    mqttConfig.topicPing = strdup(tmp.c_str());
    mqttConfig.service_sub_ping = new Adafruit_MQTT_Subscribe(mqttConfig.mqttPtr, mqttConfig.topicPing);

    tmp = DEV_PREFIX MQTT_SUB_HB;
    mqttConfig.topicHeartbeat = strdup(tmp.c_str());
    mqttConfig.service_sub_heartbeat = new Adafruit_MQTT_Subscribe(mqttConfig.mqttPtr, mqttConfig.topicHeartbeat);

    tmp = DEV_PREFIX MQTT_SUB_HEATER;
    mqttConfig.topicHeater = strdup(tmp.c_str());
    mqttConfig.service_sub_heater = new Adafruit_MQTT_Subscribe(mqttConfig.mqttPtr, mqttConfig.topicHeater);

    tmp = DEV_PREFIX MQTT_SUB_SENSOR;
    mqttConfig.topicSensor = strdup(tmp.c_str());
    mqttConfig.service_sub_sensor = new Adafruit_MQTT_Subscribe(mqttConfig.mqttPtr, mqttConfig.topicSensor);

    tmp = DEV_PREFIX MQTT_XUB_FLAGS;
    mqttConfig.topicFlags = strdup(tmp.c_str());
    mqttConfig.service_sub_flags = new Adafruit_MQTT_Subscribe(mqttConfig.mqttPtr, mqttConfig.topicFlags);
    mqttConfig.service_pub_flags = new Adafruit_MQTT_Publish(mqttConfig.mqttPtr, mqttConfig.topicFlags);

    tmp = DEV_PREFIX MQTT_PUB_LIGHT;
    mqttConfig.topicLight = strdup(tmp.c_str());
    mqttConfig.service_pub_light = new Adafruit_MQTT_Publish(mqttConfig.mqttPtr, mqttConfig.topicLight);

    tmp = DEV_PREFIX MQTT_PUB_TEMPERATURE;
    mqttConfig.topicTemperature = strdup(tmp.c_str());
    mqttConfig.service_pub_temperature = new Adafruit_MQTT_Publish(mqttConfig.mqttPtr, mqttConfig.topicTemperature);

    tmp = DEV_PREFIX MQTT_PUB_HUMIDITY;
    mqttConfig.topicHumidity = strdup(tmp.c_str());
    mqttConfig.service_pub_humidity = new Adafruit_MQTT_Publish(mqttConfig.mqttPtr, mqttConfig.topicHumidity);

    // TickerScheduler
    const uint32_t oneSec = 1000;
    ts.sched(mqtt1SecTick, oneSec);
    ts.sched(mqtt1MinTick, oneSec * 60);
    ts.sched(mqtt10MinTick, oneSec * 60 * 10);
}

void myMqttLoop()
{
    yield(); // make esp happy

    if (!checkWifiConnected())
        return;
    ArduinoOTA.handle();
    if (!checkMqttConnected())
        return;

    Adafruit_MQTT_Client &mqtt = *mqttConfig.mqttPtr;

    // Listen for updates on any subscribed MQTT feeds
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription()))
    {
        const char *message = 0;

        if (subscription == mqttConfig.service_sub_ping)
        {
            message = (const char *)subscription->lastread;
            if (strncmp(message, "periodic", 8) == 0)
            {
                // treat periodic pings as a sign that no response is needed
                // feed the reboot dog watch, so it will not take effect for a bit longer
#ifdef DEBUG
                Serial.printf("got ping: %s (feeding the watch dog)\n", message);
#endif
                state.minutes_since_periodic_ping = 0;
            }
            else
            {
#ifdef DEBUG
                Serial.printf("got ping: %s\n", message);
#endif
                // send operstate, just as a sign of pong
                sendOperState();
                sendLightSensor();
                sendTemperatureSensor();
            }
        }
        else if (subscription == mqttConfig.service_sub_heartbeat)
        {
            message = (const char *)subscription->lastread;
            parseOnOffToggle(MQTT_SUB_HB, message, clearDisableHb, setDisableHb, toggleDisableHb); // off ==> disable ==> set
            sendOperState();
        }
        else if (subscription == mqttConfig.service_sub_heater)
        {
            message = (const char *)subscription->lastread;
            parseOnOffToggle(MQTT_SUB_HEATER, message, setEnableHeater, clearEnableHeater, toggleEnableHeater);
            sendOperState();
        }
        else if (subscription == mqttConfig.service_sub_sensor)
        {
            message = (const char *)subscription->lastread;
            parseOnOffToggle(MQTT_SUB_SENSOR, message, clearDisableSensor, setDisableSensor, toggleDisableSensor); // off ==> disable ==> set
            sendOperState();
        }
        else if (subscription == mqttConfig.service_sub_flags)
        {
            message = (const char *)subscription->lastread;
            char *_end;
            const uint64_t newFlags = strtoull(message, &_end, 0) & 0xffffffff;
            // std::istringstream iss(message);
            // iss >> newFlags;

            if (state.flags == newFlags)
                return; // noop
#ifdef DEBUG
            char buff[17];
            Serial.printf("settting flags as %s -- from: 0x", message);
            snprintf(buff, sizeof(buff), "%016llx", state.flags);
            Serial.printf("%s to: 0x", buff);
            snprintf(buff, sizeof(buff), "%016llx", newFlags);
            Serial.println(buff);
#endif
            setFlags(state.flags, newFlags);
            sendOperState();
        }
        else
        {
#ifdef DEBUG
            Serial.printf("got unexpected msg on subscription: %s\n", subscription->topic);
#endif
        }
    }
}

static bool checkWifiConnected()
{
    static bool otaBegun = false;
    static bool lastWifiConnected = false;

    const bool currConnected = WiFi.status() == WL_CONNECTED;
    Adafruit_MQTT_Client &mqtt = *mqttConfig.mqttPtr;

    if (lastWifiConnected != currConnected)
    {

        if (currConnected)
        {
#ifdef DEBUG
            Serial.print("WiFi connected\nIP address: ");
            Serial.println(WiFi.localIP());
#endif

            // idem potent. If it fails, this is a game stopper...
            if (!mqtt.subscribe(mqttConfig.service_sub_ping) ||
                !mqtt.subscribe(mqttConfig.service_sub_heartbeat) ||
                !mqtt.subscribe(mqttConfig.service_sub_heater) ||
                !mqtt.subscribe(mqttConfig.service_sub_sensor) ||
                !mqtt.subscribe(mqttConfig.service_sub_flags))
            {
                gameOver("Fatal: unable to subscribe to mqtt");
            }

            if (!otaBegun)
            {
                otaBegun = true;
                ArduinoOTA.begin();
#ifdef DEBUG
                Serial.printf("Ready to perform OTA update via port %u\n", OTA_PORT);
#endif
            }
        }
        else
        {
#ifdef DEBUG
            Serial.println("WiFi disconnected");
#endif
            // assume mqtt is not connected
            mqttState.lastMqttConnected = false;
        }

        lastWifiConnected = currConnected;
    }

    return currConnected;
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
static bool checkMqttConnected()
{
    // not attempt to reconnect if there are reconnect ticks outstanding
    if (mqttState.reconnectTicks > 0)
        return false;

    Adafruit_MQTT_Client &mqtt = *mqttConfig.mqttPtr;
    const bool currMqttConnected = mqtt.connected();

    // noop?
    if (mqttState.lastMqttConnected && currMqttConnected)
        return true;

    digitalWrite(PIN_MQTT_LED, currMqttConnected ? PIN_MQTT_LED_ON : PIN_MQTT_LED_OFF);
    mqttState.lastMqttConnected = currMqttConnected;

    if (currMqttConnected)
    {
#ifdef DEBUG
        Serial.println("MQTT Connected!");
#endif
        // Don't send oper_state. Let mqtt admin dictate what the initial values should
        // be from broker
        // sendOperState();
        sendLightSensor();
        sendTemperatureSensor();
    }
    else
    {
#ifdef DEBUG
        Serial.print("MQTT is connecting... ");
#endif

        // Note: The connect call can block for up to 6 seconds.
        //       when mqtt is out... be aware.
        const int8_t ret = mqtt.connect();
        if (ret != 0)
        {
#ifdef DEBUG
            Serial.println(mqtt.connectErrorString(ret));
#endif
            mqtt.disconnect();

            // do not attempt to connect before a few ticks
            mqttState.reconnectTicks = random(defaultMqttReconnect / 4, defaultMqttReconnect + 1);
        }
        else
        {
#ifdef DEBUG
            Serial.println("done.");
#endif
        }
    }

    return currMqttConnected;
}

static void mqtt1SecTick()
{
    static int lastSentLightSensorValue = state.lightSensor;
    static float lastSentTemperature = state.temperature;
    static float lastSentHumidity = state.humidity;

    if (mqttState.reconnectTicks > 0)
        --mqttState.reconnectTicks;

#ifdef DEBUG
    if (!isMqttConnected())
    {
        Serial.print("mqtt1SecTick");
        Serial.print(" reconnectTics: ");
        Serial.print(mqttState.reconnectTicks, DEC);
        Serial.print(" mqtt_connected: ");
        Serial.print(mqttState.lastMqttConnected ? "yes" : "no");
        Serial.println("");
    }
#endif

    const int lightSensorDiff = lastSentLightSensorValue - state.lightSensor;
    if (abs(lightSensorDiff) >= 256)
    {
        lastSentLightSensorValue = state.lightSensor;
#ifdef DEBUG
        Serial.print("detected big change in light sensor. Sending update ");
        Serial.println(lastSentLightSensorValue, DEC);
#endif
        sendLightSensor();
    }

    const float temperatureDiff = lastSentTemperature - state.temperature;
    const float humidityDiff = lastSentHumidity - state.humidity;
    if (abs(temperatureDiff) >= 1 || abs(humidityDiff) >= 1)
    {
        lastSentTemperature = state.temperature;
        lastSentHumidity = state.humidity;

#ifdef DEBUG
        Serial.print("detected change temp/humidity. Sending ");
        Serial.print((int)lastSentHumidity, DEC);
        Serial.print(" % ");
        Serial.print((int)lastSentTemperature, DEC);
        Serial.print(" F");
        Serial.println("");
#endif
        sendTemperatureSensor();
    }
}

static void mqtt1MinTick()
{
#ifdef NEED_PERIODIC_PINGS
    // If it has been too long since we got a periodic ping, it is time for a game over.
    // Note that periodic pings are expected to take place as slow as 10 minutes, so we
    // shall be generous here.
    if (state.minutes_since_periodic_ping > 21)
    {
        gameOver("Too long without a periodic ping event");
        return;
    }
#endif

    state.minutes_uptime += 1;
    state.minutes_since_periodic_ping += 1;
}

static void mqtt10MinTick()
{
#ifdef DEBUG
    Serial.println("mqtt10MinTick -- sending gratuitous state");
#endif
    // grauitous
    sendOperState();
    sendLightSensor();
    sendTemperatureSensor();
}

void sendOperState()
{
    if (!isMqttConnected())
        return;

    // Lower 32 bits: flags
    // Upper 32 bits: minutes_uptime
    uint64_t operState = (uint64_t)state.minutes_uptime;
    operState <<= 32;
    operState |= state.flags & 0xffffffff;

    char buff[19];
    snprintf(buff, sizeof(buff), "0x%016llx", operState);

    if (!mqttConfig.service_pub_flags->publish(buff))
    {
#ifdef DEBUG
        Serial.println("Unable to publish stateflags");
#endif
        mqttConfig.mqttPtr->disconnect();
    }
}

static bool isMqttConnected()
{
    return mqttState.lastMqttConnected;
}

void sendLightSensor()
{
    if (!isMqttConnected())
        return;
    if (getFlag(state.flags, stateFlagsDisableSensor))
        return;

    if (!mqttConfig.service_pub_light->publish((uint32_t)state.lightSensor))
    {
#ifdef DEBUG
        Serial.println("Unable to publish light sensor value");
#endif
        mqttConfig.mqttPtr->disconnect();
    }
}

void sendTemperatureSensor()
{
    if (!isMqttConnected())
        return;
    if (getFlag(state.flags, stateFlagsDisableSensor))
        return;

    if (!mqttConfig.service_pub_temperature->publish((int32_t)state.temperature) || // signed!
        !mqttConfig.service_pub_humidity->publish((uint32_t)state.humidity))
    {
#ifdef DEBUG
        Serial.println("Unable to publish temperature sensor value");
#endif
        mqttConfig.mqttPtr->disconnect();
    }
}
