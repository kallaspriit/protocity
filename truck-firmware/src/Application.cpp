#include "Application.hpp"

Application::Application(int port) :
    SocketApplication(port)
{}

void Application::setupBefore() {
    log("setting up pin-modes");

    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(CHARGE_DETECTION_PIN, INPUT);
}

float Application::getBatteryVoltage() {
    int reading = analogRead(BATTERY_VOLTAGE_PIN);

    float actualVoltage = calculateAdcVoltage(
        reading,
        MAX_ADC_READING_VALUE,
        MAX_ADC_READING_VOLTAGE,
        BATTERY_VOLTAGE_DIVIDER_RESISTOR_1,
        BATTERY_VOLTAGE_DIVIDER_RESISTOR_2,
        BATTERY_VOLTAGE_CALIBRATION_MULTIPLIER
    );

    return actualVoltage;
}

Application::BatteryChargeState Application::getBatteryChargeState() {
    int chargingState = digitalRead(CHARGE_DETECTION_PIN);

    if (chargingState == HIGH) {
        return BatteryChargeState::CHARGE_STATE_NOT_CHARGING;
    } else {
        return BatteryChargeState::CHARGE_STATE_CHARGING;
    }
}
