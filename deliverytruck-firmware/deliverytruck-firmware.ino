
#include <ESP8266WiFi.h>
#include "WebSocketClient.h"
#include "Commander.h"

//websocket stuff
WebSocketClient webSocketClient;
HardwareSerial *serial = &Serial;
Commander commander(serial);

const int CHARGING_LED = 0;
const float CHARGING_VOLTAGE = 3.8;
const char* ssid = "Stagnationlab";
const char* password = "purgisupp";

unsigned long previousMillis = 0;
const long interval = 250;

// configure websockets
char WS_HOST[]                = "10.220.20.78";
char WS_PATH[]                = "/";
const int WS_PORT             = 3000;

WiFiClient client;
float voltage = 0.0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  digitalWrite(LED_BUILTIN, HIGH); // onboard led põlema connectimise ajaks, debug
  while (WiFi.status() != WL_CONNECTED) {
    
    delay(500);
    
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

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
    digitalWrite(LED_BUILTIN, LOW); // kustutame onboard ledi ära
  } else {
    serial->println("handshake failed");
    while (1) {
      // Hang on failure
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(CHARGING_LED, OUTPUT);
  digitalWrite(CHARGING_LED, LOW);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  setup_wifi();
}

// the loop function runs over and over again forever
void loop() {
  blinkLED(); 
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
    }
  } else {
    serial->println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
    }
  }
}

bool blinkOn = false;
bool charging = false;
void blinkLED() {
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    voltage = getBatteryVoltage();
    if (voltage > CHARGING_VOLTAGE) {
      if (!charging) {
        charging = true;
        digitalWrite(CHARGING_LED, HIGH);  // paneme lihtsalt põlema, vilkumine ebaühtlane
        String data = "truck:charging:true";
        webSocketClient.sendData(data);
       
      }
      //if (blinkOn) {
      //  digitalWrite(CHARGING_LED, LOW);     
      //} else {
      //  digitalWrite(CHARGING_LED, HIGH);
      //}
      //blinkOn = !blinkOn;
    } else {
      if (charging) {
        String data = "truck:charging:false";
        charging = false;
        digitalWrite(CHARGING_LED, LOW);
        webSocketClient.sendData(data);          
      }
    } 
  }
}
float getBatteryVoltage() {
  int reading = analogRead(A0);
  float resistor1 = 8175.0; // measured value of a 8.2k resistor
  float resistor2 = 1985.0f; // measured value of a 2k resistor
  float calibrationMultiplier = 1.053f; // multimeter-measured voltage / reported voltage
  
  int maxReading = 1023;
  float maxReadingVoltage = 1.0f;
  float sensedVoltage = ((float)reading / (float)maxReading) * maxReadingVoltage * calibrationMultiplier;
  float actualVoltage = sensedVoltage / (resistor2 / (resistor1 + resistor2));

  return actualVoltage;
}

void sendBatteryVoltage(String cl) {
  if (!client.connected()) {
    return;
  }
  if(cl == "truck") {
    voltage = getBatteryVoltage();
    String data = "battery:" + cl + ":" + String(voltage);
    webSocketClient.sendData(data);   
  }
}
void handleCommand(String command, String parameters[], int parameterCount) {
  if (command == "get-battery-voltage" && parameterCount == 1) {
    
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
