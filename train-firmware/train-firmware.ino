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
const int LOG_BUFFER_LENGTH = 128;

// behaviour config
const float OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM = 18.0f;
const unsigned long SPEED_DECISION_INTERVAL = 10;
const unsigned long BRAKE_DURATION = 250;

// dependencies
WiFiServer server(SERVER_PORT);
WiFiClient client;
Commander commander(&Serial);
MCP320X adc(ADC_SLAVE_SELECT_PIN);

// runtime info
int motorSpeed = 0;
int targetSpeed = 0;
bool isBraking = false;
bool stopAfterBrake = false;
unsigned long brakeStartTime = 0;
unsigned long lastSpeedDecisionTime = 0;
char receiveBuffer[RECEIVE_BUFFER_SIZE];
int receiveLength = 0;
bool wasClientConnected = false;
float obstacleDistance = 0.0f;
bool wasObstacleDetected = false;

// configure resources
void setup() {
  setupSerial();
  setupPinModes();
  setupAdc();
  setupMotorController();
  setupWifiConnection();
  setupServer();

  //Serial.printf("test %s (%d) %.2f\n", "priit", 29, 15.1234567890f);
  log("test2 %s (%d)", "priit", 29);
}

void setupSerial() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
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
  log("setting up wifi network");

  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);
  wifiManager.autoConnect();

  log("wifi connection established");

  // show some diagnostics information
  //WiFi.printDiag(Serial);
}

void setupServer() {
  log("setting up server.. ");
  
  // start tcp socket server
  server.begin();
  
  log("server started on %s:%d", WiFi.localIP().toString().c_str(), SERVER_PORT);
}

void loop() {
  loopCommander();
  loopServer();
  loopMotorController();
}

void loopCommander() {
  while (commander.gotCommand()) {
    handleCommand(commander.command, commander.parameters, commander.parameterCount);
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
  //String message = client.readStringUntil('\n');

  // read data from the client
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
  Serial.println(message);
  
  commander.parseCommand(message);
  handleCommand(commander.command, commander.parameters, commander.parameterCount);
}

void sendMessage(String message) {
  if (!client.connected()) {
    Serial.print("> ");
    Serial.print(message);
    Serial.println(" (no client connected)");

    return;
  }

  Serial.print(">");
  Serial.println(message);
  
  client.print(message + String("\n"));
}

void handleCommand(String command, String parameters[], int parameterCount) {
  if (command == "led" && parameterCount == 1) {
    int state = parameters[0].toInt() == 1 ? HIGH : LOW;

    setDebugLed(state);

    if (state) {
      log("turning debug led on");
    } else {
      log("turning debug led off");
    }

    sendLedState();
  } else if (command == "toggle-led" && parameterCount == 0) {
    toggleDebugLed();

    log("toggling debug led");

    sendLedState();
  } else if (command == "get-led-state" && parameterCount == 0) {
    sendLedState();
  } else if (command == "set-speed" && parameterCount == 1) {
    targetSpeed = min(max(parameters[0].toInt(), -100), 100);

    applyMotorSpeed();
  } else if (command == "get-speed" && parameterCount == 0) {
    sendMotorSpeed();
  } else if (command == "brake") {
    log("braking");

    brakeMotor();
    
    stopAfterBrake = true;
  } else if (command == "get-motor-state" && parameterCount == 0) {
    sendMotorSpeed();
  } else if (command == "get-battery-voltage" && parameterCount == 0) {
    sendBatteryVoltage();
  } else if (command == "get-obstacle-distance") {
    sendObstacleDistance();
  } else {
    log("got command '%s' with %d parameters:", command.c_str(), parameterCount);

    for (int i = 0; i < parameterCount; i++) {
      log("  %d: %s", i, parameters[i].c_str());
    }

    sendMessage("unsupported command");
  }
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

void setMotorSpeed(int speed, bool sendState = true) {
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

  if (sendState) {
    sendMotorSpeed();
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
      setMotorSpeed(targetSpeed > 0 ? -100 : 100, false);
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
    } else {
      log("applying target speed of %d%%", targetSpeed);
    }
    
    setMotorSpeed(targetSpeed);
  }
}

void sendLedState() {
  sendMessage("led:" + String(digitalRead(DEBUG_LED_PIN)));
}

void sendMotorSpeed() {
  sendMessage("speed:" + String(motorSpeed) + ":" + String(targetSpeed));
}

void sendObstacleDetectedEvent(float distance) {
  sendMessage("obstacle-detected:" + String(distance));
}

void sendObstacleClearedEvent() {
  sendMessage("obstacle-cleared");
}

void sendBatteryVoltage() {
  float voltage = getBatteryVoltage();

  log("battery voltage: %sV", String(voltage).c_str());

  sendMessage("battery:" + vehicle + ":" + String(voltage));
}

void sendObstacleDistance() {
  float distance = getObstacleDistance();

  log("obstacle distance: %s cm", String(distance).c_str());

  sendMessage("obstacle-distance:" + String(distance));
}

void log(char *fmt, ...){
    char buf[LOG_BUFFER_LENGTH];
    va_list args;
    va_start (args, fmt );
    vsnprintf(buf, LOG_BUFFER_LENGTH, fmt, args);
    va_end (args);
    Serial.print(String("# ") + String(buf) + String("\n"));
}
