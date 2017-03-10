#ifndef SOCKETAPPLICATION_H
#define SOCKETAPPLICATION_H

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.hpp>
#include <Commander.hpp>

#include <Arduino.h>
#include <stdarg.h>

// Helpful links
// Common pins            https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/generic/common.h
// Sparkfun thing pins    https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/thing/pins_arduino.h
// Hookup guide           https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/all

class SocketApplication {

public:
    SocketApplication(int port);

    virtual void setup();
    virtual void loop();

protected:

    // battery charge states
    enum BatteryChargeState {
        CHARGE_STATE_UNKNOWN,
        CHARGE_STATE_CHARGING,
        CHARGE_STATE_NOT_CHARGING
    };

    // should be implemented by the child class
    virtual String getVersion() = 0;

    // generic setup
    virtual void setupDebugLed();
    virtual void setupSerial();
    virtual void setupGreeting();
    virtual void setupWifiConnection();
    virtual void setupServer();

    // application setup before and after generic
    virtual void setupBefore() {};
    virtual void setupAfter() {};

    // application setup before and after generic
    virtual void loopBefore() {};
    virtual void loopAfter() {};

    // called on every loop
    virtual void loopSerial();
    virtual void loopServer();

    // battery handling
    virtual void loopBatteryMonitor();
    virtual float getBatteryVoltage() = 0;
    virtual BatteryChargeState getBatteryChargeState() = 0;

    // handle events
    virtual void handleClientConnected();
    virtual void handleClientDataAvailable();
    virtual void handleClientDisconnected();
    virtual void handleMessage(String message);

    // handle commands
    virtual void handleRawCommand(int requestId, String command, String parameters[], int parameterCount);
    virtual void handleCommand(int requestId, String command, String parameters[], int parameterCount);
    virtual void handlePingCommand(int requestId, String parameters[], int parameterCount);
    virtual void handleVersionCommand(int requestId, String parameters[], int parameterCount);
    virtual void handleGetBatteryVoltageCommand(int requestId, String parameters[], int parameterCount);
    virtual void handleIsChargingCommand(int requestId, String parameters[], int parameterCount);
    virtual void handleUnsupportedCommand(int requestId, String command, String parameters[], int parameterCount);

    // send messages and events
    virtual void sendMessage(const char *fmt, ...);
    virtual void sendMessage(String message);
    virtual void sendSuccessMessage(int requestId);
    virtual void sendSuccessMessage(int requestId, int value);
    virtual void sendSuccessMessage(int requestId, int value1, int value2);
    virtual void sendSuccessMessage(int requestId, String info);
    virtual void sendSuccessMessage(int requestId, String info1, String info2);
    virtual void sendSuccessMessage(int requestId, String info1, String info2, String info3);
    virtual void sendErrorMessage(int requestId);
    virtual void sendEventMessage(String event);
    virtual void sendEventMessage(String event, String info);
    virtual void sendEventMessage(String event, String info1, String info2);
    virtual void sendEventMessage(String event, String info1, String info2, String info3);
    virtual void sendErrorMessage(int requestId, String reason);

    // callback for battery state change
    virtual void onBatteryStateChanged(BatteryChargeState state, float voltage) {};

    // response senders
    virtual void sendBatteryVoltage(int requestId);
    virtual void sendIsCharging(int requestId);

    // debug led handling
    virtual void toggleDebugLed();
    virtual void setDebugLed(int state);

    // helpers
    virtual float calculateAdcVoltage(int reading, int maxReading, float maxReadingVoltage, float resistor1, float resistor2, float calibrationMultiplier);
    virtual int getBatteryChargePercentage(float voltage);

    // log utility
    virtual void log(const char *fmt, ...);

    // server config
    int port;

    // configure pins
    static const int DEBUG_LED_PIN = LED_BUILTIN; // should be pin 5

    // buffers
    static const int RECEIVE_BUFFER_SIZE = 1024;
    static const int LOG_BUFFER_SIZE = 128;
    static const int COMMAND_BUFFER_SIZE = 128;

    // battery monitor
    const unsigned long BATTERY_MONITOR_INTERVAL_MS = 500;
    const float BATTERY_VOLTAGE_CHANGE_THRESHOLD = 0.05f;

    // dependencies
    WiFiServer server;
    WiFiClient client;
    Commander commander;

    // runtime info
    char receiveBuffer[RECEIVE_BUFFER_SIZE];
    char logBuffer[LOG_BUFFER_SIZE];
    char commandBuffer[COMMAND_BUFFER_SIZE];
    int receiveLength = 0;
    int commandLength = 0;
    bool wasClientConnected = false;
    unsigned long lastBatteryMonitorCheckTime = 0;
    BatteryChargeState lastBatteryChargeState = BatteryChargeState::CHARGE_STATE_UNKNOWN;
    float lastBatteryVoltage = 0.0f;
};

#endif
