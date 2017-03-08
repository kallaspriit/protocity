#ifndef APPLICATION_H
#define APPLICATION_H

#include <SocketApplication.hpp>
#include <MCP320X.hpp>
#include <SPI.h>
#include <stdarg.h>

class Application : public SocketApplication {

public:
    Application(int port);

private:
    // provide version number
    virtual String getVersion() { return "2.10.0"; };

    // setup additional dependecies
    virtual void setupBefore();

    // battery handling
    virtual float getBatteryVoltage();
    virtual BatteryChargeState getBatteryChargeState();

    // pins config
    const int BATTERY_VOLTAGE_PIN = A0;
    const int CHARGE_DETECTION_PIN = 4;

    // analog-to-digital converter config
    const float MAX_ADC_READING_VOLTAGE = 1.0f; // Vcc/Vref pin
    const int MAX_ADC_READING_VALUE = 1023;

    // battery voltage detection config
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_1 = 8200.0f;       // between input and output
    const float BATTERY_VOLTAGE_DIVIDER_RESISTOR_2 = 2400.0f;       // between input and ground
    const float BATTERY_VOLTAGE_CALIBRATION_MULTIPLIER = 1.062f;     // multimeter-measured voltage / reported voltage
};

#endif
