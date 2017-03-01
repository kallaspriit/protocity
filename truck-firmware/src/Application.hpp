#ifndef APPLICATION_H
#define APPLICATION_H

#include <SocketApplication.hpp>
#include <MCP320X.hpp>
#include <SPI.h>
#include <stdarg.h>

class Application : public SocketApplication {

public:
    Application(int port);

    virtual void setup();
    virtual void loop();

private:

    // battery charge states
    enum BatteryChargeState {
        CHARGE_STATE_UNKNOWN,
        CHARGE_STATE_CHARGING,
        CHARGE_STATE_NOT_CHARGING
    };

    // provide version number
    virtual String getVersion() { return "2.6.0"; };

    // setup additional dependecies
    void setupPinModes();

    // application-specific loop handlers
    void loopBatteryMonitor();

    // handle commands
    virtual bool handleCommand(int requestId, String command, String parameters[], int parameterCount);

    // handle application-specific commands
    void handleGetBatteryVoltageCommand(int requestId, String parameters[], int parameterCount);
    void handleIsChargingCommand(int requestId, String parameters[], int parameterCount);

    // response senders
    void sendBatteryVoltage(int requestId);
    void sendIsCharging(int requestId);

    // battery handling
    float getBatteryVoltage();
    float calculateAdcVoltage(int reading, int maxReading, float maxReadingVoltage, float resistor1, float resistor2, float calibrationMultiplier);
    int getBatteryChargePercentage(float voltage);
    BatteryChargeState getBatteryChargeState();

    // pins config
    const int BATTERY_VOLTAGE_PIN = A0;
    const int CHARGE_DETECTION_PIN = 4;

    // analog-to-digital converter config
    const float MAX_ADC_READING_VOLTAGE = 1.0f; // Vcc/Vref pin
    const int MAX_ADC_READING_VALUE = 1023;

    // battery voltage detection config
    const unsigned long BATTERY_MONITOR_INTERVAL_MS = 500;
    const float BATTERY_VOLTAGE_CHANGE_THRESHOLD = 0.01f;
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_1 = 8200.0f;       // between input and output
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_2 = 2400.0f;       // between input and ground
    const float BATTERY_VOLTAGE_CALIBRATION_MULTIPLIER = 1.062f;     // multimeter-measured voltage / reported voltage

    // runtime info
    unsigned long lastBatteryMonitorCheckTime = 0;
    BatteryChargeState lastBatteryChargeState = BatteryChargeState::CHARGE_STATE_UNKNOWN;
    float lastBatteryVoltage = 0.0f;
};

#endif
