#include "Application.hpp"

Application::Application(int port) :
    SocketApplication(port),
    adc(ADC_SLAVE_SELECT_PIN)
{}

void Application::setup() {
    SocketApplication::setup();

    setupPinModes();
    setupAdc();
    setupMotorController();
}


void Application::setupPinModes() {
    log("setting up pin-modes");

    pinMode(MOTOR_CONTROL_PIN_A, OUTPUT);
    pinMode(MOTOR_CONTROL_PIN_B, OUTPUT);
}

void Application::setupAdc() {
    log("setting up analog to digital converter");

    SPI.begin();
    adc.begin();
}

void Application::setupMotorController() {
    log("setting up motor controller");

    // make the motor brake by default
    stopMotor();
}

void Application::loop() {
    SocketApplication::loop();

    loopMotorController();
}

void Application::loopMotorController() {
    unsigned long currentTime = millis();
    unsigned long timeSinceLastSpeedDecision = currentTime - lastSpeedDecisionTime;

    if (timeSinceLastSpeedDecision > SPEED_DECISION_INTERVAL) {
        applyMotorSpeed();

        lastSpeedDecisionTime = currentTime;
    }
}

void Application::handleClientDisconnected() {
    SocketApplication::handleClientDisconnected();

    targetSpeed = 0;
}

bool Application::handleCommand(int requestId, String command, String parameters[], int parameterCount) {
    if (SocketApplication::handleCommand(requestId, command, parameters, parameterCount)) {
        return true;
    }

    if (command == "set-speed") {
        handleSetSpeedCommand(requestId, parameters, parameterCount);
    } else if (command == "get-speed") {
        handleGetSpeedCommand(requestId, parameters, parameterCount);
    } else if (command == "get-battery-voltage") {
        handleGetBatteryVoltageCommand(requestId, parameters, parameterCount);
    } else if (command == "get-obstacle-distance") {
        handleGetObstacleDistanceCommand(requestId, parameters, parameterCount);
    } else {
        handleUnsupportedCommand(requestId, command, parameters, parameterCount);

        return false;
    }

    return true;
}

void Application::handleSetSpeedCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 1) {
        return sendErrorMessage(requestId, "expected 1 parameter, for example '1:set-speed:50'");
    }

    targetSpeed = min(max(parameters[0].toInt(), -100), 100);

    applyMotorSpeed();

    sendMotorSpeed(requestId);
}

void Application::handleGetSpeedCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 0) {
        return sendErrorMessage(requestId, "expected no parameters, for example '1:get-speed'");
    }

    sendMotorSpeed(requestId);
}

void Application::handleGetBatteryVoltageCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 0) {
        return sendErrorMessage(requestId, "expected no parameters, for example '1:get-battery-voltage'");
    }

    sendBatteryVoltage(requestId);
}

void Application::handleGetObstacleDistanceCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 0) {
        return sendErrorMessage(requestId, "expected no parameters, for example '1:get-obstacle-distance'");
    }

    sendObstacleDistance(requestId);
}

void Application::sendMotorSpeed(int requestId) {
    sendSuccessMessage(requestId, motorSpeed, targetSpeed);
}

void Application::sendBatteryVoltage(int requestId) {
    float voltage = getBatteryVoltage();
    int chargePercentage = getBatteryChargePercentage(voltage);

    sendSuccessMessage(requestId, String(voltage), String(chargePercentage));
}

void Application::sendObstacleDistance(int requestId) {
    float distance = getObstacleDistance();

    sendSuccessMessage(requestId, String(distance));
}

void Application::sendObstacleDetectedEvent(float distance) {
    sendEventMessage("obstacle-detected", String(distance));
}

void Application::sendObstacleClearedEvent() {
    sendEventMessage("obstacle-cleared", String(obstacleDetectedFrames));
}

float Application::getBatteryVoltage() {
    int reading = adc.read12(adc.SINGLE_CH0);

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

float Application::getObstacleDistance() {
    int reading = adc.read12(adc.SINGLE_CH1);

    float voltage = calculateAdcVoltage(reading, MAX_ADC_READING_VALUE, MAX_ADC_READING_VOLTAGE, 0, 1, 1.0);
    float distance = max(min(13.0f * pow(voltage, -1), 30.0f), 4.0f);

    return distance;
}

bool Application::isObstacleDetected() {
    obstacleDistance = getObstacleDistance();

    return obstacleDistance < OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM;
}

void Application::setMotorSpeed(int speed) {
    if (speed == motorSpeed) {
        return;
    }

    speed = min(max(speed, -100), 100);

    int analogOutValue = (int)(((float)abs(speed) / 100.0f) * (float)ANALOG_MAX_VALUE);

    log("setting motor to %s at %d%% speed", speed > 0 ? "move forward" : speed < 0 ? "move in reverse" : "stop", speed);

    motorSpeed = speed;

    // set outputs
    if (speed == 0) {
        analogWrite(MOTOR_CONTROL_PIN_A, ANALOG_MAX_VALUE);
        analogWrite(MOTOR_CONTROL_PIN_B, ANALOG_MAX_VALUE);
    } else if (speed > 0) {
        analogWrite(MOTOR_CONTROL_PIN_A, ANALOG_MAX_VALUE);
        analogWrite(MOTOR_CONTROL_PIN_B, ANALOG_MAX_VALUE - analogOutValue);
    } else {
        analogWrite(MOTOR_CONTROL_PIN_A, ANALOG_MAX_VALUE - analogOutValue);
        analogWrite(MOTOR_CONTROL_PIN_B, ANALOG_MAX_VALUE);
    }

    sendEventMessage("speed-changed", String(motorSpeed), String(targetSpeed));
}

void Application::stopMotor() {
    setMotorSpeed(0);
}

void Application::brakeMotor() {
    isBraking = true;
    brakeStartTime = millis();
    applyMotorSpeed();
}

void Application::applyMotorSpeed() {
    if (isBraking) {
        unsigned long currentTime = millis();
        unsigned long brakingDuration = currentTime - brakeStartTime;

        if (brakingDuration > BRAKE_DURATION) {
            if (stopAfterBrake == true) {
                targetSpeed = 0;
            }

            isBraking = false;
            stopAfterBrake = false;
            stopMotor();
        } else {
            setMotorSpeed(targetSpeed > 0 ? -100 : 100);
        }

        return;
    }

    if (isObstacleDetected()) {
        if (!wasObstacleDetected) {
            wasObstacleDetected = true;

            handleObstacleDetected();
        }

        obstacleDetectedFrames++;
    } else {
        if (wasObstacleDetected) {
            handleObstacleCleared();

            wasObstacleDetected = false;
            obstacleDetectedFrames = 0;
        } else if (motorSpeed != targetSpeed) {
            log("applying target speed of %d%%", targetSpeed);

            setMotorSpeed(targetSpeed);
        }
    }
}

void Application::handleObstacleDetected() {
    // stop the train
    if (motorSpeed != 0) {
        log("obstacle detected, stopping train");

        stopMotor();
        //brakeMotor();
    }

    sendObstacleDetectedEvent(obstacleDistance);
}

void Application::handleObstacleCleared() {
    sendObstacleClearedEvent();

    // restart the train if it was moving before
    if (targetSpeed != 0) {
        log("obstacle cleared, resuming target speed of %d%%", targetSpeed);

        setMotorSpeed(targetSpeed);
    }
}
