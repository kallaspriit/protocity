#include "Application.hpp"

Application::Application(int port) :
    SocketApplication(port)
{}

void Application::setup() {
    SocketApplication::setup();

    setupPinModes();
}


void Application::setupPinModes() {
    log("setting up pin-modes");

    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
    pinMode(CHARGE_DETECTION_PIN, INPUT);
}

void Application::loop() {
    SocketApplication::loop();

    loopBatteryMonitor();
}

void Application::loopBatteryMonitor() {
    unsigned long currentTime = millis();
    unsigned long timeSinceLastCheck = currentTime - lastBatteryMonitorCheckTime;

    // perform the check periodically without stopping the loop
    if (timeSinceLastCheck < BATTERY_MONITOR_INTERVAL_MS) {
        return;
    }

    BatteryChargeState batteryChargeState = getBatteryChargeState();
    float batteryVoltage = getBatteryVoltage();
    int chargePercentage = getBatteryChargePercentage(batteryVoltage);

    // check for charge state change
    if (batteryChargeState != lastBatteryChargeState) {
        if (batteryChargeState == BatteryChargeState::CHARGE_STATE_CHARGING) {
            sendEventMessage("battery-charging", String(batteryVoltage), String(chargePercentage));
        } else if (batteryChargeState == BatteryChargeState::CHARGE_STATE_NOT_CHARGING) {
            sendEventMessage("battery-not-charging", String(batteryVoltage), String(chargePercentage));
        }

        lastBatteryChargeState = batteryChargeState;
    }

    // check for battery voltage change
    if (fabs(batteryVoltage - lastBatteryVoltage) >= BATTERY_VOLTAGE_CHANGE_THRESHOLD) {
        sendEventMessage("battery-voltage-changed", String(batteryVoltage), String(chargePercentage));

        lastBatteryVoltage = batteryVoltage;
    }

    lastBatteryMonitorCheckTime = currentTime;
}

bool Application::handleCommand(int requestId, String command, String parameters[], int parameterCount) {
    if (SocketApplication::handleCommand(requestId, command, parameters, parameterCount)) {
        return true;
    }

    if (command == "get-battery-voltage") {
        handleGetBatteryVoltageCommand(requestId, parameters, parameterCount);
    } else if (command == "is-charging") {
        handleIsChargingCommand(requestId, parameters, parameterCount);
    } else {
        handleUnsupportedCommand(requestId, command, parameters, parameterCount);

        return false;
    }

    return true;
}

void Application::handleGetBatteryVoltageCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 0) {
        return sendErrorMessage(requestId, "expected no parameters, for example '1:get-battery-voltage'");
    }

    sendBatteryVoltage(requestId);
}

void Application::handleIsChargingCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 0) {
        return sendErrorMessage(requestId, "expected no parameters, for example '1:is-charging'");
    }

    sendIsCharging(requestId);
}

void Application::sendBatteryVoltage(int requestId) {
    float voltage = getBatteryVoltage();
    int chargePercentage = getBatteryChargePercentage(voltage);

    sendSuccessMessage(requestId, String(voltage), String(chargePercentage));
}

void Application::sendIsCharging(int requestId) {
    int chargingState = digitalRead(CHARGE_DETECTION_PIN);

    sendSuccessMessage(requestId, chargingState == HIGH ? 0 : 1);
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

float Application::calculateAdcVoltage(int reading, int maxReading, float maxReadingVoltage, float resistor1, float resistor2, float calibrationMultiplier) {
    float sensedVoltage = ((float)reading / (float)maxReading) * maxReadingVoltage * calibrationMultiplier;
    float actualVoltage = sensedVoltage / (resistor2 / (resistor1 + resistor2));

    return actualVoltage;
}

int Application::getBatteryChargePercentage(float voltage) {
    if (voltage >= 4.20f) {
        return 100;
    } else if (voltage >= 4.15f) {
        return 90;
    } else if (voltage >= 4.10f) {
        return 80;
    } else if (voltage >= 4.05f) {
        return 70;
    } else if (voltage >= 4.00f) {
        return 60;
    } else if (voltage >= 3.95f) {
        return 50;
    } else if (voltage >= 3.90f) {
        return 40;
    } else if (voltage >= 3.85f) {
        return 30;
    } else if (voltage >= 3.80f) {
        return 20;
    } else if (voltage >= 3.70f) {
        return 10;
    } else if (voltage >= 3.60f) {
        return 5;
    } else {
        return 1;
    }
}

Application::BatteryChargeState Application::getBatteryChargeState() {
    int chargingState = digitalRead(CHARGE_DETECTION_PIN);

    if (chargingState == HIGH) {
        return BatteryChargeState::CHARGE_STATE_NOT_CHARGING;
    } else {
        return BatteryChargeState::CHARGE_STATE_CHARGING;
    }
}
