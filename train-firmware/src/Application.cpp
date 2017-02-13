#include "Application.hpp"

Application::Application() :
    server(SERVER_PORT),
    adc(ADC_SLAVE_SELECT_PIN)
{}

void Application::setup() {
    setupSerial();
    setupPinModes();
    setupAdc();
    setupMotorController();
    setupWifiConnection();
    setupBatteryMonitor();
    setupServer();
}

void Application::setupSerial() {
    Serial.begin(115200);
    delay(100);
    Serial.print("\n\n");
}

void Application::setupPinModes() {
    log("setting up pin-modes");

    pinMode(DEBUG_LED_PIN, OUTPUT);
    pinMode(MOTOR_CONTROL_PIN_A, OUTPUT);
    pinMode(MOTOR_CONTROL_PIN_B, OUTPUT);
    pinMode(BATTERY_VOLTAGE_PIN, INPUT);
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

void Application::setupWifiConnection() {
    log("setting up wifi connection");

    WiFiManager wifiManager;
    wifiManager.setDebugOutput(false);
    wifiManager.autoConnect();

    log("wifi connection established");

    // show some diagnostics information
    //WiFi.printDiag(Serial);
}

void Application::setupBatteryMonitor() {
    initialBatteryVoltage = getBatteryVoltage();

    log("setting up battery monitor, initial voltage: %sV", String(initialBatteryVoltage).c_str());
}

void Application::setupServer() {
    log("setting up server.. ");

    // start tcp socket server
    server.begin();

    log("server started on %s:%d", WiFi.localIP().toString().c_str(), SERVER_PORT);
}

void Application::loop() {
    loopSerial();
    loopServer();
    loopMotorController();
}

void Application::loopSerial() {
    while (Serial.available() > 0) {
        char character = Serial.read();

        if (character == '\n') {
            commandBuffer[commandLength++] = '\0';

            handleMessage(String(commandBuffer));

            commandLength = 0;
        } else {
            commandBuffer[commandLength++] = character;
        }

        // avoid buffer overflow
        if (commandLength == COMMAND_BUFFER_SIZE -1) {
            break;
        }
    }
}

void Application::loopServer() {
    if (!client.connected()) {
        if (wasClientConnected) {
            handleClientDisconnected();

            wasClientConnected = false;
        }

        client = server.available();

        if (client.connected()) {
            handleClientConnected();

            wasClientConnected = true;
        }
    } else {
        if (client.available() > 0) {
            handleClientDataAvailable();
        }
    }
}

void Application::loopMotorController() {
    unsigned long currentTime = millis();
    unsigned long timeSinceLastSpeedDecision = currentTime - lastSpeedDecisionTime;

    if (timeSinceLastSpeedDecision > SPEED_DECISION_INTERVAL) {
        applyMotorSpeed();

        lastSpeedDecisionTime = currentTime;
    }
}

void Application::handleClientConnected() {
    log("client connected, remote ip: %s", client.remoteIP().toString().c_str());
}

void Application::handleClientDataAvailable() {
    while (client.available()) {
        char character = client.read();

        if (character == '\n') {
            receiveBuffer[receiveLength++] = '\0';

            handleMessage(String(receiveBuffer));

            //client.flush();

            receiveLength = 0;
        } else {
            receiveBuffer[receiveLength++] = character;
        }

        // avoid buffer overflow
        if (receiveLength == RECEIVE_BUFFER_SIZE -1) {
            break;
        }
    }
}

void Application::handleClientDisconnected() {
    log("client disconnected, stopping motor");

    targetSpeed = 0;
}

void Application::handleMessage(String message) {
    if (message.length() == 0) {
        return;
    }

    Serial.print("< ");
    Serial.print(message + String("\n"));

    commander.parseCommand(message);

    if (commander.isValid) {
        handleCommand(commander.id, commander.command, commander.parameters, commander.parameterCount);
    } else {
        log("got incomplete command message '%s', expected something like 1:command:arg1:arg2", message.c_str());

        sendErrorMessage(commander.id, "incomplete command");
    }
}

void Application::handleCommand(int requestId, String command, String parameters[], int parameterCount) {
    if (command == "ping") {
        handlePingCommand(requestId, parameters, parameterCount);
    } else if (command == "set-led") {
        handleSetLedCommand(requestId, parameters, parameterCount);
    } else if (command == "get-led") {
        handleGetLedCommand(requestId, parameters, parameterCount);
    } else if (command == "toggle-led") {
        handleToggleLedCommand(requestId, parameters, parameterCount);
    } else if (command == "set-speed") {
        handleSetSpeedCommand(requestId, parameters, parameterCount);
    } else if (command == "get-speed") {
        handleGetSpeedCommand(requestId, parameters, parameterCount);
    } else if (command == "get-battery-voltage") {
        handleGetBatteryVoltageCommand(requestId, parameters, parameterCount);
    } else if (command == "get-obstacle-distance") {
        handleGetObstacleDistanceCommand(requestId, parameters, parameterCount);
    } else {
        handleUnsupportedCommand(requestId, command, parameters, parameterCount);
    }
}

void Application::handlePingCommand(int requestId, String parameters[], int parameterCount) {
    sendSuccessMessage(requestId, "pong");
}

void Application::handleSetLedCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 1) {
        return sendErrorMessage(requestId, "expected 1 parameter, for example '1:set-led:1'");
    }

    int state = parameters[0].toInt() == 1 ? HIGH : LOW;

    setDebugLed(state);

    if (state) {
        log("turning debug led on");
    } else {
        log("turning debug led off");
    }

    sendLedState(requestId);
}

void Application::handleGetLedCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 0) {
        return sendErrorMessage(requestId, "expected no parameters, for example '1:get-led'");
    }

    sendLedState(requestId);
}

void Application::handleToggleLedCommand(int requestId, String parameters[], int parameterCount) {
    if (parameterCount != 0) {
        return sendErrorMessage(requestId, "expected no parameters, for example '1:toggle-led'");
    }

    toggleDebugLed();

    log("toggling debug led");

    sendLedState(requestId);
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

void Application::handleUnsupportedCommand(int requestId, String command, String parameters[], int parameterCount) {
    log("got command #%d '%s' with %d parameters:", requestId, command.c_str(), parameterCount);

    for (int i = 0; i < parameterCount; i++) {
        log("  %d: %s", i, parameters[i].c_str());
    }

    sendErrorMessage(requestId, "unsupported command");
}

void Application::sendMessage(char *fmt, ...) {
    va_list args;
    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);

    if (client.connected()) {
        client.print(String(logBuffer) + String("\n"));

        Serial.print(String("> ") + String(logBuffer) + String("\n"));
    } else {
        Serial.print(String("> ") + String(logBuffer) + String(" (no client connected)\n"));
    }
}

void Application::sendMessage(String message) {
    if (client.connected()) {
        client.print(message + String("\n"));

        Serial.print(String("> ") + message + String("\n"));
    } else {
        Serial.print(String("> ") + message + String(" (no client connected)\n"));
    }
}

void Application::sendSuccessMessage(int requestId) {
    sendMessage("%d:OK", requestId);
}

void Application::sendSuccessMessage(int requestId, int value) {
    sendMessage("%d:OK:%d", requestId, value);
}

void Application::sendSuccessMessage(int requestId, int value1, int value2) {
    sendMessage("%d:OK:%d:%d", requestId, value1, value2);
}

void Application::sendSuccessMessage(int requestId, String info) {
    sendMessage("%d:OK:%s", requestId, info.c_str());
}

void Application::sendSuccessMessage(int requestId, String info1, String info2) {
    sendMessage("%d:OK:%s:%s", requestId, info1.c_str(), info2.c_str());
}

void Application::sendErrorMessage(int requestId) {
    sendMessage("%d:ERROR", requestId);
}

void Application::sendEventMessage(String event) {
    sendMessage("0:%s", event.c_str());
}

void Application::sendEventMessage(String event, String info) {
    sendMessage("0:%s:%s", event.c_str(), info.c_str());
}

void Application::sendEventMessage(String event, String info1, String info2) {
    sendMessage("0:%s:%s:%s", event.c_str(), info1.c_str(), info2.c_str());
}

void Application::sendErrorMessage(int requestId, String reason) {
    sendMessage("%d:ERROR:%s", requestId, reason.c_str());
}

void Application::sendLedState(int requestId) {
    sendSuccessMessage(requestId, digitalRead(DEBUG_LED_PIN) == HIGH ? 1 : 0);
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

void Application::log(char *fmt, ...) {
    va_list args;
    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);
    Serial.print(String("# ") + String(logBuffer) + String("\n"));
}

float Application::getBatteryVoltage() {
    float resistor1 = 8200.0; // between input and output
    float resistor2 = 15000.0f; // between input and ground
    float calibrationMultiplier = 0.99f; // multimeter-measured voltage / reported voltage

    int reading = adc.read12(adc.SINGLE_CH0);

    float actualVoltage = calculateAdcVoltage(reading, MAX_ADC_READING_VALE, MAX_ADC_READING_VOLTAGE, resistor1, resistor2, calibrationMultiplier);

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

    float voltage = calculateAdcVoltage(reading, MAX_ADC_READING_VALE, MAX_ADC_READING_VOLTAGE, 0, 1, 1.0);
    float distance = max(min(13.0f * pow(voltage, -1), 30.0f), 4.0f);

    return distance;
}

bool Application::isObstacleDetected() {
    obstacleDistance = getObstacleDistance();

    return obstacleDistance < OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM;
}

void Application::toggleDebugLed() {
    setDebugLed(digitalRead(DEBUG_LED_PIN) == HIGH ? LOW : HIGH);
}

void Application::setDebugLed(int state) {
    digitalWrite(DEBUG_LED_PIN, state == HIGH ? HIGH : LOW);
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
    /*
    // stop the train
    if (motorSpeed != 0) {
        log("obstacle detected, stopping train");

        stopMotor();
        //brakeMotor();
    }
    */

    sendObstacleDetectedEvent(obstacleDistance);
}

void Application::handleObstacleCleared() {
    sendObstacleClearedEvent();

    /*
    // restart the train if it was moving before
    if (targetSpeed != 0) {
        log("obstacle cleared, resuming target speed of %d%%", targetSpeed);

        setMotorSpeed(targetSpeed);
    }
    */
}
