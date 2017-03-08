#include "Application.hpp"

Application::Application(int port) :
    SocketApplication(port),
    adc(ADC_SLAVE_SELECT_PIN)
{}

void Application::setupBefore() {
    setupPinModes();
}

void Application::setupGreeting() {
    Serial.print(String("\n\n### Train v") + getVersion() + String(" ###\n"));
}

void Application::setupAfter() {
    setupChargeDetection();
    setupAdc();
    setupMotorController();
}

void Application::setupPinModes() {
    log("setting up pin-modes");

    pinMode(MOTOR_CONTROL_PIN_A, OUTPUT);
    pinMode(MOTOR_CONTROL_PIN_B, OUTPUT);

}

void Application::setupChargeDetection() {
    log("setting up charge detection");

    pinMode(CHARGE_DETECTION_PIN, INPUT);
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

void Application::loopAfter() {
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

void Application::handleCommand(int requestId, String command, String parameters[], int parameterCount) {
    if (command == "set-speed") {
        handleSetSpeedCommand(requestId, parameters, parameterCount);
    } else if (command == "get-speed") {
        handleGetSpeedCommand(requestId, parameters, parameterCount);
    } else if (command == "get-obstacle-distance") {
        handleGetObstacleDistanceCommand(requestId, parameters, parameterCount);
    } else {
        handleUnsupportedCommand(requestId, command, parameters, parameterCount);
    }
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

void Application::handleGetObstacleDistanceCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 0) {
        return sendErrorMessage(requestId, "expected no parameters, for example '1:get-obstacle-distance'");
    }

    sendObstacleDistance(requestId);
}

void Application::sendMotorSpeed(int requestId) {
    sendSuccessMessage(requestId, motorSpeed, targetSpeed);
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
        MAX_ADC_READ_VALUE,
        MAX_ADC_READ_VOLTAGE,
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

float Application::getObstacleDistance() {
    int reading = adc.read12(adc.SINGLE_CH1);

    float voltage = calculateAdcVoltage(reading, MAX_ADC_READ_VALUE, MAX_ADC_READ_VOLTAGE, 0, 1, 1.0);
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

    int analogOutValue = (int)(((float)abs(speed) / 100.0f) * (float)MAX_ANALOG_WRITE_VALUE);

    log("setting motor to %s at %d%% speed", speed > 0 ? "move forward" : speed < 0 ? "move in reverse" : "stop", speed);

    motorSpeed = speed;

    // set outputs
    if (speed == 0) {
        analogWrite(MOTOR_CONTROL_PIN_A, MAX_ANALOG_WRITE_VALUE);
        analogWrite(MOTOR_CONTROL_PIN_B, MAX_ANALOG_WRITE_VALUE);
    } else if (speed > 0) {
        analogWrite(MOTOR_CONTROL_PIN_A, MAX_ANALOG_WRITE_VALUE);
        analogWrite(MOTOR_CONTROL_PIN_B, MAX_ANALOG_WRITE_VALUE - analogOutValue);
    } else {
        analogWrite(MOTOR_CONTROL_PIN_A, MAX_ANALOG_WRITE_VALUE - analogOutValue);
        analogWrite(MOTOR_CONTROL_PIN_B, MAX_ANALOG_WRITE_VALUE);
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
