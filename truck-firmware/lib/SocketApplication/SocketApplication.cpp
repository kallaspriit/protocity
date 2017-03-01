#include "SocketApplication.hpp"

SocketApplication::SocketApplication(int port) :
    port(port),
    server(port)
{}

void SocketApplication::setup() {
    setupDebugLed();
    setupSerial();
    setupWifiConnection();
    setupServer();
}

void SocketApplication::setupDebugLed() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void SocketApplication::setupSerial() {
    Serial.begin(115200);
    delay(100);
    Serial.print("\n\n");
}

void SocketApplication::setupWifiConnection() {
    log("setting up wifi connection");

    WiFiManager wifiManager;
    wifiManager.setDebugOutput(false);
    wifiManager.autoConnect();

    log("wifi connection established");

    // show some diagnostics information
    //WiFi.printDiag(Serial);
}

void SocketApplication::setupServer() {
    log("setting up server.. ");

    // start tcp socket server
    server.begin();

    log("server started on %s:%d", WiFi.localIP().toString().c_str(), port);
}

void SocketApplication::loop() {
    loopSerial();
    loopServer();
}

void SocketApplication::loopSerial() {
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

void SocketApplication::loopServer() {
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

void SocketApplication::handleClientConnected() {
    log("client connected, remote ip: %s", client.remoteIP().toString().c_str());
}

void SocketApplication::handleClientDataAvailable() {
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

void SocketApplication::handleClientDisconnected() {
    log("client disconnected");
}

void SocketApplication::handleMessage(String message) {
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

bool SocketApplication::handleCommand(int requestId, String command, String parameters[], int parameterCount) {
    if (command == "ping") {
        handlePingCommand(requestId, parameters, parameterCount);
    } else if (command == "version") {
        handleVersionCommand(requestId, parameters, parameterCount);
    } else {
        return false;
    }

    return true;
}

void SocketApplication::handlePingCommand(int requestId, String parameters[], int parameterCount) {
    sendSuccessMessage(requestId, "pong");
}

void SocketApplication::handleVersionCommand(int requestId, String parameters[], int parameterCount) {
    sendSuccessMessage(requestId, getVersion());
}

void SocketApplication::handleUnsupportedCommand(int requestId, String command, String parameters[], int parameterCount) {
    log("got command #%d '%s' with %d parameters:", requestId, command.c_str(), parameterCount);

    for (int i = 0; i < parameterCount; i++) {
        log("  %d: %s", i, parameters[i].c_str());
    }

    sendErrorMessage(requestId, "unsupported command");
}

void SocketApplication::sendMessage(const char *fmt, ...) {
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

void SocketApplication::sendMessage(String message) {
    if (client.connected()) {
        client.print(message + String("\n"));

        Serial.print(String("> ") + message + String("\n"));
    } else {
        Serial.print(String("> ") + message + String(" (no client connected)\n"));
    }
}

void SocketApplication::sendSuccessMessage(int requestId) {
    sendMessage("%d:OK", requestId);
}

void SocketApplication::sendSuccessMessage(int requestId, int value) {
    sendMessage("%d:OK:%d", requestId, value);
}

void SocketApplication::sendSuccessMessage(int requestId, int value1, int value2) {
    sendMessage("%d:OK:%d:%d", requestId, value1, value2);
}

void SocketApplication::sendSuccessMessage(int requestId, String info) {
    sendMessage("%d:OK:%s", requestId, info.c_str());
}

void SocketApplication::sendSuccessMessage(int requestId, String info1, String info2) {
    sendMessage("%d:OK:%s:%s", requestId, info1.c_str(), info2.c_str());
}

void SocketApplication::sendSuccessMessage(int requestId, String info1, String info2, String info3) {
    sendMessage("%d:OK:%s:%s:%s", requestId, info1.c_str(), info2.c_str(), info3.c_str());
}

void SocketApplication::sendErrorMessage(int requestId) {
    sendMessage("%d:ERROR", requestId);
}

void SocketApplication::sendEventMessage(String event) {
    sendMessage("0:%s", event.c_str());
}

void SocketApplication::sendEventMessage(String event, String info) {
    sendMessage("0:%s:%s", event.c_str(), info.c_str());
}

void SocketApplication::sendEventMessage(String event, String info1, String info2) {
    sendMessage("0:%s:%s:%s", event.c_str(), info1.c_str(), info2.c_str());
}

void SocketApplication::sendEventMessage(String event, String info1, String info2, String info3) {
    sendMessage("0:%s:%s:%s:%s", event.c_str(), info1.c_str(), info2.c_str(), info3.c_str());
}

void SocketApplication::sendErrorMessage(int requestId, String reason) {
    sendMessage("%d:ERROR:%s", requestId, reason.c_str());
}

void SocketApplication::log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);
    Serial.print(String("# ") + String(logBuffer) + String("\n"));
}

void SocketApplication::toggleDebugLed() {
    setDebugLed(digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH);
}

void SocketApplication::setDebugLed(int state) {
    digitalWrite(LED_BUILTIN, state == HIGH ? HIGH : LOW);
}