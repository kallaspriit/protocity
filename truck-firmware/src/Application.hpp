#ifndef APPLICATION_H
#define APPLICATION_H

#include <SocketApplication.hpp>
#include <MCP320X.hpp>
#include <SPI.h>

class Application : public SocketApplication {

public:
    Application(int port);

private:
    // provide version number
    virtual String getVersion() { return "2.22.0"; };

    // setup additional dependecies
    virtual void setupBefore();
    virtual void setupGreeting();

    // battery handling
    virtual float getBatteryVoltage();
    virtual BatteryChargeState getBatteryChargeState();
    virtual void onBatteryStateChanged(BatteryChargeState state, float voltage);

    // pins config
    const int BATTERY_VOLTAGE_PIN = A0;
    const int BATTERY_CHARGE_STATE_PIN = 0;
    const int CHARGE_DETECTION_PIN = 4; // actual charge IC status input
    const int CHARGE_PRESENCE_PIN = 13; // charge voltage presence input

    // analog-to-digital converter config
    const float MAX_ADC_READING_VOLTAGE = 1.0f; // Vcc/Vref pin
    const int MAX_ADC_READING_VALUE = 1023;

    // battery voltage detection config
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_1 = 8200.0f;       // between input and output
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_2 = 2400.0f;       // between input and ground
    const float BATTERY_VOLTAGE_CALIBRATION_MULTIPLIER = 1.062f;     // multimeter-measured voltage / reported voltage
};

#endif
