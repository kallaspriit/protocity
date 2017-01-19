#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <SPI.h>

// Helpful links
// Common pins            https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/generic/common.h
// Sparkfun thing pins    https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/thing/pins_arduino.h
// Hookup guide           https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/all

#include "WebSocketClient.h"
#include "Commander.h"
#include "MCP320X.h"

// configure wify
const char WIFI_SSID[]        = "Stagnationlab";
const char WIFI_PASSWORD[]    = "purgisupp";

// configure websockets
char WS_HOST[]                = "10.220.20.140";
char WS_PATH[]                = "/";
const int WS_PORT             = 3000;

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

// behaviour config
const float OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM = 18.0f;
const unsigned long SPEED_DECISION_INTERVAL = 10;
const unsigned long BRAKE_DURATION = 250;

// dependencies
WebSocketClient webSocketClient;
WiFiClient client;
HardwareSerial *serial = &Serial;
Commander commander(serial);
MCP320X adc(ADC_SLAVE_SELECT_PIN);

// runtime info
int motorSpeed = 0;
int targetSpeed = 0;
bool isBraking = false;
bool stopAfterBrake = false;
unsigned long brakeStartTime = 0;
unsigned long lastSpeedDecisionTime = 0;

// configure resources
void setup() {
  // initialize serial for debugging
  serial->begin(115200);
  delay(10);

  // setup pins
  pinMode(DEBUG_LED_PIN, OUTPUT);
  pinMode(MOTOR_CONTROL_PIN_A, OUTPUT);
  pinMode(MOTOR_CONTROL_PIN_B, OUTPUT);
  pinMode(BATTERY_VOLTAGE_PIN, INPUT);

  // setup default pin states
  digitalWrite(DEBUG_LED_PIN, HIGH);

  // make the motor brake by default
  stopMotor();

  // connect to the wifi network
  serial->print("connecting to wifi network: ");
  serial->println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // wait for the connection, blink the debug led
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    serial->print(".");
    toggleDebugLed();
  }

  // kill the led once connected, lights up again once connected to websocket
  setDebugLed(LOW);

  // send wifi connection info to the console
  serial->print("connected to wifi network, my ip: ");
  serial->println(WiFi.localIP());

  WiFi.printDiag(Serial);

  // give the chip some time before connecting to websocket
  delay(500);

  // send websocket info to console
  serial->print("connecting to ws://");
  serial->print(WS_HOST);
  serial->print(":");
  serial->print(WS_PORT);
  serial->println(WS_PATH);

  // connect to the websocket server
  if (client.connect(WS_HOST, WS_PORT)) {
    serial->println("connected to websocket");
  } else {
    serial->println("connecting to websocket failed");
    while (1) {
      // hang on failure, watchdog will reboot it
    }
  }

  // perform the websocket handshake
  serial->println("performing websocket handshake");

  // configure handshake
  webSocketClient.path = WS_PATH;
  webSocketClient.host = WS_HOST;

  // perform the handshake
  if (webSocketClient.handshake(client)) {
    serial->println("handshake was successful");
  } else {
    serial->println("handshake failed");
    while (1) {
      // Hang on failure
    }
  }

  // initialize SPI and adc
  SPI.begin();
  adc.begin();

  // all done with the setup, show solid led
  serial->println("setup complete");
  setDebugLed(HIGH);
}

void loop() {
  //Serial.print(".");
  
  while (commander.gotCommand()) {
    handleCommand(commander.command, commander.parameters, commander.parameterCount);
  }

  if (client.connected()) {
    String data;

    webSocketClient.getData(data);

    if (data.length() > 0) {
      serial->print("< ");
      serial->println(data);

      commander.parseCommand(data);
      handleCommand(commander.command, commander.parameters, commander.parameterCount);

      /*
        if (data == "toggle-led") {
        toggleDebugLed();

        data = "led:" + String(digitalRead(BUILTIN_LED));
        webSocketClient.sendData(data);
        }
      */
    }
  } else {
    serial->println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
    }
  }

  unsigned long currentTime = millis();
  unsigned long timeSinceLastSpeedDecision = currentTime - lastSpeedDecisionTime;

  if (timeSinceLastSpeedDecision > SPEED_DECISION_INTERVAL) {
    applyMotorSpeed();

    lastSpeedDecisionTime = currentTime;
  }
}

void handleCommand(String command, String parameters[], int parameterCount) {
  if (command == "led" && parameterCount == 1) {
    int state = parameters[0].toInt() == 1 ? HIGH : LOW;

    setDebugLed(state);

    serial->println(state ? "turning debug led on" : "turning debug led off");

    sendLedState();
  } else if (command == "toggle-led" && parameterCount == 0) {
    toggleDebugLed();

    serial->println("toggling debug led");

    sendLedState();
  } else if (command == "get-led-state" && parameterCount == 0) {
    sendLedState();
  } else if (command == "motor" && parameterCount == 1) {
    targetSpeed = min(max(parameters[0].toInt(), -100), 100);

    applyMotorSpeed();
  } else if (command == "brake") {
    serial->println("braking");

    brakeMotor();
    
    stopAfterBrake = true;
  } else if (command == "get-motor-state" && parameterCount == 0) {
    sendMotorState();
  } else if (command == "get-battery-voltage" && parameterCount == 1) {
    sendBatteryVoltage(parameters[0]);
  } else if (command == "get-obstacle-distance") {
    sendObstacleDistance();
  } else {
    serial->print("Got command '");
    serial->print(command);
    serial->print("' with ");
    serial->print(parameterCount);
    serial->println(" parameters: ");

    for (int i = 0; i < parameterCount; i++) {
      serial->print("  > ");
      serial->print(i);
      serial->print(": ");
      serial->println(parameters[i]);
    }
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
  
  serial->print("setting motor to ");
  serial->print(speed > 0 ? "move forward" : speed < 0 ? "move in reverse" : "stop");
  serial->print(" at ");
  serial->print(speed);
  serial->println("% speed");

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
    sendMotorState();
  }
}

void stopMotor() {
  serial->println("stopping motor");

  setMotorSpeed(0);
}

void brakeMotor() {
  isBraking = true;
  brakeStartTime = millis();
  applyMotorSpeed();
}

void sendLedState() {
  if (!client.connected()) {
    return;
  }
  
  String data = "led:" + String(digitalRead(DEBUG_LED_PIN));
  webSocketClient.sendData(data);
}

void sendMotorState() {
  if (!client.connected()) {
    return;
  }

  String data = "motor:" + String(motorSpeed) + ":" + String(targetSpeed);
  webSocketClient.sendData(data);
}

void sendBatteryVoltage(String vehicle) {
  if (!client.connected()) {
    return;
  }
  
  if (vehicle != "train") {
    return;
  }
  
  float voltage = getBatteryVoltage();

  serial->print("battery voltage: ");
  serial->print(voltage);
  serial->println("V");

  String data = "battery:" + vehicle + ":" + String(voltage);
  webSocketClient.sendData(data);
}

void sendObstacleDistance() {
  if (!client.connected()) {
    return;
  }
  
  float distance = getObstacleDistance();

  serial->print("obstacle distance: ");
  serial->print(distance);
  serial->println("cm");

  String data = "obstacle-distance:" + String(distance);
  webSocketClient.sendData(data);
}

float calculateAdcVoltage(int reading, int maxReading, float maxReadingVoltage, float resistor1, float resistor2, float calibrationMultiplier) {
  float sensedVoltage = ((float)reading / (float)maxReading) * maxReadingVoltage * calibrationMultiplier;
  float actualVoltage = sensedVoltage / (resistor2 / (resistor1 + resistor2));

  return actualVoltage;
}

bool isObstacleDetected() {
  float obstacleDistanceCm = getObstacleDistance();

  return obstacleDistanceCm < OBSTACLE_DETECTED_DISTANCE_THRESHOLD_CM;
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
      Serial.println("obstacle detected, stopping train");
  
      //stopMotor();
      brakeMotor();
    }
  } else if (motorSpeed != targetSpeed) {
    Serial.print("no obstacle detected, matching motor speed to target of ");
    Serial.print(targetSpeed);
    Serial.println("%");
    
    setMotorSpeed(targetSpeed);
  }
}

