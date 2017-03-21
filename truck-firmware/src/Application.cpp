#include "Application.hpp"

Application::Application(int port) :
    SocketApplication(port)
{}

void Application::setupBefore() {
    log("setting up pin-modes");

    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(BATTERY_CHARGE_STATE_PIN, OUTPUT);
    pinMode(CHARGE_DETECTION_PIN, INPUT);
    pinMode(CHARGE_PRESENCE_PIN, INPUT);

    digitalWrite(BATTERY_CHARGE_STATE_PIN, HIGH); // not charging initially
}

void Application::setupGreeting() {
    Serial.print(String("\n\n### Truck v") + getVersion() + String(" ###\n"));
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
    bool isChargeVoltagePresent = digitalRead(CHARGE_PRESENCE_PIN) == HIGH;
    bool isActuallyCharging = digitalRead(CHARGE_DETECTION_PIN) == LOW;

    //log("charge voltage is %s, the truck is actually %s", isChargeVoltagePresent ? "present" : "not present", isActuallyCharging ? "charging" : "not charging");

    if (isChargeVoltagePresent || isActuallyCharging) {
        return BatteryChargeState::CHARGE_STATE_CHARGING;
    } else {
        return BatteryChargeState::CHARGE_STATE_NOT_CHARGING;
    }
}

void Application::onBatteryStateChanged(BatteryChargeState state, float voltage) {
    bool isCharging = state == BatteryChargeState::CHARGE_STATE_CHARGING;

    log("battery is now %s", isCharging ? "charging" : "not charging");

    digitalWrite(BATTERY_CHARGE_STATE_PIN, isCharging ? HIGH : LOW);
}
