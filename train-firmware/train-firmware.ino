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

// dependencies
WebSocketClient webSocketClient;
WiFiClient client;
HardwareSerial *serial = &Serial;
Commander commander(serial);
MCP320X adc(ADC_SLAVE_SELECT_PIN);

// runtime info
String motorDirection = "stop";
int motorSpeed = 0;

// configure resources
void setup() {
  // initialize serial for debugging
  serial->begin(9600);
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

  // wait to fully let the client disconnect
  //delay(3000);

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
  } else if (command == "motor" && parameterCount >= 1 && parameterCount <= 2) {
    String direction = parameters[0];
    int speed = 100;

    // extract speed if available
    if (parameterCount == 2) {
      speed = min(max(parameters[1].toInt(), 0), 100);
    }

    // apply motor speed
    if (direction == "forward") {
      setMotorSpeed(speed);
    } else if (direction == "reverse") {
      setMotorSpeed(-speed);
    } else if (direction == "stop") {
      speed = 0;
      
      stopMotor();
    } else {
      serial->println("invalid motor direction provided, expected either forward, reverse or stop");

      return;
    }

    // update last state
    motorDirection = direction;
    motorSpeed = speed;

    sendMotorState();
  } else if (command == "get-motor-state" && parameterCount == 0) {
    sendMotorState();
  } else if (command == "get-battery-voltage" && parameterCount == 1) {
    float voltage = getBatteryVoltage();

    serial->print("battery voltage: ");
    serial->print(voltage);
    serial->println("V");
    
    sendBatteryVoltage(parameters[0]);
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
  int maxReading = 4095;
  float maxReadingVoltage = 3.3f; // Vcc/Vref pin

  int reading = adc.read12(adc.SINGLE_CH0);

  float actualVoltage = calculateAdcVoltage(reading, maxReading, maxReadingVoltage, resistor1, resistor2, calibrationMultiplier);

  return actualVoltage;
}

void toggleDebugLed() {
  setDebugLed(digitalRead(DEBUG_LED_PIN) == HIGH ? LOW : HIGH);
}

void setDebugLed(int state) {
  digitalWrite(DEBUG_LED_PIN, state == HIGH ? HIGH : LOW);
}

void stopMotor() {
  serial->println("stopping motor");
  
  analogWrite(MOTOR_CONTROL_PIN_A, ANALOG_MAX_VALUE);
  analogWrite(MOTOR_CONTROL_PIN_B, ANALOG_MAX_VALUE);
}

void setMotorSpeed(int speed) {
  speed = min(max(speed, -100), 100);
  int analogOutValue = (int)(((float)abs(speed) / 100.0f) * (float)ANALOG_MAX_VALUE);
  
  serial->print("setting motor to ");
  serial->print(speed > 0 ? "move forward" : "reverse");
  serial->print(" at ");
  serial->print(speed);
  serial->println("% speed");

  // set outputs
  if (speed > 0) {
    analogWrite(MOTOR_CONTROL_PIN_A, ANALOG_MAX_VALUE);
    analogWrite(MOTOR_CONTROL_PIN_B, ANALOG_MAX_VALUE - analogOutValue);
  } else {
    analogWrite(MOTOR_CONTROL_PIN_A, ANALOG_MAX_VALUE - analogOutValue);
    analogWrite(MOTOR_CONTROL_PIN_B, ANALOG_MAX_VALUE);
  }
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
  
  String data = "motor:" + motorDirection + ":" + String(motorSpeed);
  webSocketClient.sendData(data);
}

void sendBatteryVoltage(String cl) {
  if (!client.connected()) {
    return;
  }
  if (cl == "train") {
    float voltage = getBatteryVoltage();
 
    String data = "battery:" + cl + ":" + String(voltage);
    webSocketClient.sendData(data);    
  }
}

float calculateAdcVoltage(int reading, int maxReading, float maxReadingVoltage, float resistor1, float resistor2, float calibrationMultiplier) {
  float sensedVoltage = ((float)reading / (float)maxReading) * maxReadingVoltage * calibrationMultiplier;
  float actualVoltage = sensedVoltage / (resistor2 / (resistor1 + resistor2));

  return actualVoltage;
}

