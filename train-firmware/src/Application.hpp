#ifndef APPLICATION_H
#define APPLICATION_H

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <SPI.h>
#include <stdarg.h>

#include <WiFiManager.hpp>
#include <Commander.hpp>
#include <MCP320X.hpp>

// Helpful links
// Common pins            https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/generic/common.h
// Sparkfun thing pins    https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/thing/pins_arduino.h
// Hookup guide           https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/all

class Application {

public:
    Application();

    void setup();
    void loop();

private:
    // configure server
    static const int SERVER_PORT = 8080;

    // configure pins
    static const int DEBUG_LED_PIN       = LED_BUILTIN; // should be pin 5
    static const int MOTOR_CONTROL_PIN_A = 0;
    static const int MOTOR_CONTROL_PIN_B = 4;
    static const int BATTERY_VOLTAGE_PIN = A0;
    static const int ADC_SLAVE_SELECT_PIN = 2;

    // environment config
    static const int ANALOG_MAX_VALUE = 1023;
    const float MAX_ADC_READING_VOLTAGE = 3.3f; // Vcc/Vref pin
    static const int MAX_ADC_READING_VALE = 4095;
    static const int RECEIVE_BUFFER_SIZE = 1024;
    static const int LOG_BUFFER_SIZE = 128;
    static const int COMMAND_BUFFER_SIZE = 128;

    // behaviour config
    const float OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM = 11.5f;
    static const unsigned long SPEED_DECISION_INTERVAL = 10;
    static const unsigned long BRAKE_DURATION = 250;

    // dependencies
    WiFiServer server;
    WiFiClient client;
    Commander commander;
    MCP320X adc;

    // runtime info
    int motorSpeed = 0;
    int targetSpeed = 0;
    bool isBraking = false;
    bool stopAfterBrake = false;
    unsigned long brakeStartTime = 0;
    unsigned long lastSpeedDecisionTime = 0;
    char receiveBuffer[RECEIVE_BUFFER_SIZE];
    char logBuffer[LOG_BUFFER_SIZE];
    char commandBuffer[COMMAND_BUFFER_SIZE];
    int receiveLength = 0;
    int commandLength = 0;
    bool wasClientConnected = false;
    float obstacleDistance = 0.0f;
    int obstacleDetectedFrames = 0;
    bool wasObstacleDetected = false;
    float initialBatteryVoltage = 0.0f;

    // setup dependecies
    void setupSerial();
    void setupPinModes();
    void setupAdc();
    void setupMotorController();
    void setupWifiConnection();
    void setupBatteryMonitor();
    void setupServer();

    // called on every loop
    void loopSerial();
    void loopServer();
    void loopMotorController();

    // handle events
    void handleClientConnected();
    void handleClientDataAvailable();
    void handleClientDisconnected();
    void handleMessage(String message);

    // handle commands
    void handleCommand(int requestId, String command, String parameters[], int parameterCount);
    void handlePingCommand(int requestId, String parameters[], int parameterCount);
    void handleSetLedCommand(int requestId, String parameters[], int parameterCount);
    void handleGetLedCommand(int requestId, String parameters[], int parameterCount);
    void handleToggleLedCommand(int requestId, String parameters[], int parameterCount);
    void handleSetSpeedCommand(int requestId, String parameters[], int parameterCount);
    void handleGetSpeedCommand(int requestId, String parameters[], int parameterCount);
    void handleGetBatteryVoltageCommand(int requestId, String parameters[], int parameterCount);
    void handleGetObstacleDistanceCommand(int requestId, String parameters[], int parameterCount);
    void handleUnsupportedCommand(int requestId, String command, String parameters[], int parameterCount);

    // send messages and events
    void sendMessage(char *fmt, ...);
    void sendMessage(String message);
    void sendSuccessMessage(int requestId);
    void sendSuccessMessage(int requestId, int value);
    void sendSuccessMessage(int requestId, int value1, int value2);
    void sendSuccessMessage(int requestId, String info);
    void sendSuccessMessage(int requestId, String info1, String info2);
    void sendErrorMessage(int requestId);
    void sendEventMessage(String event);
    void sendEventMessage(String event, String info);
    void sendEventMessage(String event, String info1, String info2);
    void sendErrorMessage(int requestId, String reason);

    // send state to client
    void sendLedState(int requestId);
    void sendMotorSpeed(int requestId);
    void sendBatteryVoltage(int requestId);
    void sendObstacleDistance(int requestId);
    void sendObstacleDetectedEvent(float distance);
    void sendObstacleClearedEvent();

    // battery handling
    float getBatteryVoltage();
    float calculateAdcVoltage(int reading, int maxReading, float maxReadingVoltage, float resistor1, float resistor2, float calibrationMultiplier);
    int getBatteryChargePercentage(float voltage);

    // obstacle handling
    float getObstacleDistance();
    bool isObstacleDetected();

    // debug led handling
    void toggleDebugLed();
    void setDebugLed(int state);

    // motor handling
    void setMotorSpeed(int speed);
    void stopMotor();
    void brakeMotor();
    void applyMotorSpeed();

    // log utility
    void log(char *fmt, ...);
};

#endif
