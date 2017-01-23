#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <SPI.h>
#include <stdarg.h>

#include "WiFiManager.h"
#include "Commander.h"
#include "MCP320X.h"

// Helpful links
// Common pins            https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/generic/common.h
// Sparkfun thing pins    https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/thing/pins_arduino.h
// Hookup guide           https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/all

// configure server
const int SERVER_PORT = 8080;

// configure pins
const int DEBUG_LED_PIN       = LED_BUILTIN; // should be pin 5
const int MOTOR_CONTROL_PIN_A = 0;
const int MOTOR_CONTROL_PIN_B = 4;
const int BATTERY_VOLTAGE_PIN = A0;
const int ADC_SLAVE_SELECT_PIN = 2;

// environment config
const int ANALOG_MAX_VALUE = 1023;
const float MAX_ADC_READING_VOLTAGE = 3.3f; // Vcc/Vref pin
const int MAX_ADC_READING_VALE = 4095;
const int RECEIVE_BUFFER_SIZE = 1024;
const int LOG_BUFFER_SIZE = 128;
const int COMMAND_BUFFER_SIZE = 128;

// behaviour config
const float OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM = 18.0f;
const unsigned long SPEED_DECISION_INTERVAL = 10;
const unsigned long BRAKE_DURATION = 250;

// dependencies
WiFiServer server(SERVER_PORT);
WiFiClient client;
Commander commander;
MCP320X adc(ADC_SLAVE_SELECT_PIN);

// runtime info
int motorSpeed = 0;
int targetSpeed = 0;
bool isBraking = false;
bool stopAfterBrake = false;
unsigned long brakeStartTime = 0;
unsigned long lastSpeedDecisionTime = 0;
char receiveBuffer[RECEIVE_BUFFER_SIZE];
char logBuffer[LOG_BUFFER_SIZE];
char commandBuffer[COMMAND_BUFFER_SIZE];
int receiveLength = 0;
int commandLength = 0;
bool wasClientConnected = false;
float obstacleDistance = 0.0f;
bool wasObstacleDetected = false;
float initialBatteryVoltage = 0.0f;

// configure resources
void setup() {
  setupSerial();
  setupPinModes();
  setupAdc();
  setupMotorController();
  setupWifiConnection();
  setupBatteryMonitor();
  setupServer();
}

void setupSerial() {
  Serial.begin(115200);
  delay(100);
  Serial.print("\n");
}

void setupPinModes() {
  log("setting up pin-modes");
  
  pinMode(DEBUG_LED_PIN, OUTPUT);
  pinMode(MOTOR_CONTROL_PIN_A, OUTPUT);
  pinMode(MOTOR_CONTROL_PIN_B, OUTPUT);
  pinMode(BATTERY_VOLTAGE_PIN, INPUT);
}

void setupAdc() {
  log("setting up analog to digital converter");
  
  SPI.begin();
  adc.begin();
}

void setupMotorController() {
  log("setting up motor controller");
  
  // make the motor brake by default
  stopMotor();
}

void setupWifiConnection() {
  log("setting up wifi connection");

  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);
  wifiManager.autoConnect();

  log("wifi connection established");

  // show some diagnostics information
  //WiFi.printDiag(Serial);
}

void setupBatteryMonitor() {
  initialBatteryVoltage = getBatteryVoltage();
  
  log("setting up battery monitor, initial voltage: %sV", String(initialBatteryVoltage).c_str());
}

void setupServer() {
  log("setting up server.. ");
  
  // start tcp socket server
  server.begin();
  
  log("server started on %s:%d", WiFi.localIP().toString().c_str(), SERVER_PORT);
}

void loop() {
  loopSerial();
  loopServer();
  loopMotorController();
}

void loopSerial() {
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

void loopServer() {
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

void loopMotorController() {
  unsigned long currentTime = millis();
  unsigned long timeSinceLastSpeedDecision = currentTime - lastSpeedDecisionTime;

  if (timeSinceLastSpeedDecision > SPEED_DECISION_INTERVAL) {
    applyMotorSpeed();

    lastSpeedDecisionTime = currentTime;
  }
}

void handleClientConnected() {
  log("client connected, remote ip: %s", client.remoteIP().toString().c_str());
}

void handleClientDataAvailable() {
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

void handleClientDisconnected() {
  log("client disconnected");
}

void handleMessage(String message) {
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

void handleCommand(int requestId, String command, String parameters[], int parameterCount) {
  if (command == "set-led") {
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

void handleSetLedCommand(int requestId, String parameters[], int parameterCount) {
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

void handleGetLedCommand(int requestId, String parameters[], int parameterCount) {
  if (parameterCount != 0) {
    return sendErrorMessage(requestId, "expected no parameters, for example '1:get-led'");
  }
  
  sendLedState(requestId);
}

void handleToggleLedCommand(int requestId, String parameters[], int parameterCount) {
  if (parameterCount != 0) {
    return sendErrorMessage(requestId, "expected no parameters, for example '1:toggle-led'");
  }
  
  toggleDebugLed();

  log("toggling debug led");

  sendLedState(requestId);
}

void handleSetSpeedCommand(int requestId, String parameters[], int parameterCount) {
  if (parameterCount != 1) {
    return sendErrorMessage(requestId, "expected 1 parameter, for example '1:set-speed:50'");
  }

  targetSpeed = min(max(parameters[0].toInt(), -100), 100);

  applyMotorSpeed();

  sendMotorSpeed(requestId);
}

void handleGetSpeedCommand(int requestId, String parameters[], int parameterCount) {
  if (parameterCount != 0) {
    return sendErrorMessage(requestId, "expected no parameters, for example '1:get-speed'");
  }

  sendMotorSpeed(requestId);
}

void handleGetBatteryVoltageCommand(int requestId, String parameters[], int parameterCount) {
  if (parameterCount != 0) {
    return sendErrorMessage(requestId, "expected no parameters, for example '1:get-battery-voltage'");
  }

  sendBatteryVoltage(requestId);
}

void handleGetObstacleDistanceCommand(int requestId, String parameters[], int parameterCount) {
  if (parameterCount != 0) {
    return sendErrorMessage(requestId, "expected no parameters, for example '1:get-obstacle-distance'");
  }

  sendObstacleDistance(requestId);
}

void handleUnsupportedCommand(int requestId, String command, String parameters[], int parameterCount) {
  log("got command #%d '%s' with %d parameters:", requestId, command.c_str(), parameterCount);

  for (int i = 0; i < parameterCount; i++) {
    log("  %d: %s", i, parameters[i].c_str());
  }

  sendErrorMessage(requestId, "unsupported command");
}

void sendMessage(char *fmt, ...) {
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

void sendMessage(String message) {
  if (client.connected()) {
    client.print(message + String("\n"));
    
    Serial.print(String("> ") + message + String("\n"));
  } else {
    Serial.print(String("> ") + message + String(" (no client connected)\n"));
  }
}

void sendSuccessMessage(int requestId) {
  sendMessage("%d:OK", requestId);
}

void sendSuccessMessage(int requestId, int value) {
  sendMessage("%d:OK:%d", requestId, value);
}

void sendSuccessMessage(int requestId, int value1, int value2) {
  sendMessage("%d:OK:%d:%d", requestId, value1, value2);
}

void sendSuccessMessage(int requestId, String info) {
  sendMessage("%d:OK:%s", requestId, info.c_str());
}

void sendSuccessMessage(int requestId, String info1, String info2) {
  sendMessage("%d:OK:%s:%s", requestId, info1.c_str(), info2.c_str());
}

void sendErrorMessage(int requestId) {
  sendMessage("%d:ERROR", requestId);
}

void sendEventMessage(String event) {
  sendMessage("0:%s", event.c_str());
}

void sendEventMessage(String event, String info) {
  sendMessage("0:%s:%s", event.c_str(), info.c_str());
}

void sendErrorMessage(int requestId, String reason) {
  sendMessage("%d:ERROR:%s", requestId, reason.c_str());
}

void sendLedState(int requestId) {
  sendSuccessMessage(requestId, digitalRead(DEBUG_LED_PIN) == HIGH ? 1 : 0);
}

void sendMotorSpeed(int requestId) {
  sendSuccessMessage(requestId, motorSpeed, targetSpeed);
}

void sendObstacleDetectedEvent(float distance) {
  sendEventMessage("obstacle-detected", String(distance));
}

void sendObstacleClearedEvent() {
  sendEventMessage("obstacle-cleared");
}

void sendBatteryVoltage(int requestId) {
  float voltage = getBatteryVoltage();
  int chargePercentage = getBatteryChargePercentage(voltage);

  sendSuccessMessage(requestId, String(voltage), String(chargePercentage));
}

void sendObstacleDistance(int requestId) {
  float distance = getObstacleDistance();

  sendSuccessMessage(requestId, String(distance));
}

void log(char *fmt, ...){
    va_list args;
    va_start(args, fmt );
    vsnprintf(logBuffer, LOG_BUFFER_SIZE, fmt, args);
    va_end(args);
    Serial.print(String("# ") + String(logBuffer) + String("\n"));
}

float getBatteryVoltage() {
  float resistor1 = 8200.0; // between input and output
  float resistor2 = 15000.0f; // between input and ground
  float calibrationMultiplier = 0.99f; // multimeter-measured voltage / reported voltage

  int reading = adc.read12(adc.SINGLE_CH0);

  float actualVoltage = calculateAdcVoltage(reading, MAX_ADC_READING_VALE, MAX_ADC_READING_VOLTAGE, resistor1, resistor2, calibrationMultiplier);

  return actualVoltage;
}

float getObstacleDistance() {
  int reading = adc.read12(adc.SINGLE_CH1);
  
  float voltage = calculateAdcVoltage(reading, MAX_ADC_READING_VALE, MAX_ADC_READING_VOLTAGE, 0, 1, 1.0);
  float distance = max(min(13.0f * pow(voltage, -1), 30.0f), 4.0f);

  return distance;
}

void toggleDebugLed() {
  setDebugLed(digitalRead(DEBUG_LED_PIN) == HIGH ? LOW : HIGH);
}

void setDebugLed(int state) {
  digitalWrite(DEBUG_LED_PIN, state == HIGH ? HIGH : LOW);
}

void setMotorSpeed(int speed) {
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

void stopMotor() {
  setMotorSpeed(0);
}

void brakeMotor() {
  isBraking = true;
  brakeStartTime = millis();
  applyMotorSpeed();
}

float calculateAdcVoltage(int reading, int maxReading, float maxReadingVoltage, float resistor1, float resistor2, float calibrationMultiplier) {
  float sensedVoltage = ((float)reading / (float)maxReading) * maxReadingVoltage * calibrationMultiplier;
  float actualVoltage = sensedVoltage / (resistor2 / (resistor1 + resistor2));

  return actualVoltage;
}

bool isObstacleDetected() {
  obstacleDistance = getObstacleDistance();

  return obstacleDistance < OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM;
}

int getBatteryChargePercentage(float voltage) {
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

void applyMotorSpeed() {
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
  } else if (isObstacleDetected() && targetSpeed > 0) {
    if (motorSpeed != 0 && !isBraking) {
      log("obstacle detected, stopping train");
  
      //stopMotor();
      brakeMotor();

      sendObstacleDetectedEvent(obstacleDistance);

      wasObstacleDetected = true;
    }
  } else if (motorSpeed != targetSpeed) {
    if (wasObstacleDetected) {
      sendObstacleClearedEvent();

      log("obstacle cleared, resuming target speed of %d%%", targetSpeed);

      wasObstacleDetected = false;
    } else {
      log("applying target speed of %d%%", targetSpeed);
    }
    
    setMotorSpeed(targetSpeed);
  }
}
