#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"

const int SERVER_PORT = 8080;
const int RECEIVE_BUFFER_SIZE = 1024;

WiFiServer server(SERVER_PORT);
WiFiClient client;

char receiveBuffer[RECEIVE_BUFFER_SIZE];
int messageLength = 0;
bool wasClientConnected = false;

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println("setting up server");

  WiFiManager wifiManager;
  wifiManager.setDebugOutput(false);
  wifiManager.autoConnect();

  IPAddress ip = WiFi.localIP();
  
  Serial.println("connection established, starting server");

  server.begin();
  
  Serial.print("server started on ");
  Serial.print(ip.toString());
  Serial.print(":");
  Serial.println(SERVER_PORT);
  //Serial.setDebugOutput(true);
}

void loop() {
  loopServer();
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

void handleClientConnected() {
  Serial.print("client connected, remote ip: ");
  Serial.println(client.remoteIP().toString());
}

void handleClientDataAvailable() {
  //String message = client.readStringUntil('\n');
  
  while (client.available()) {
    receiveBuffer[messageLength++] = client.read();
  }

  receiveBuffer[messageLength++] = '\0';
  
  client.flush();

  Serial.print("< ");
  Serial.println(receiveBuffer);

  messageLength = 0;
  
  sendMessage("ACK");
}

void handleClientDisconnected() {
  Serial.println("client disconnected");
}

void sendMessage(String message) {
  if (!client.connected()) {
    Serial.print("requested to send message '");
    Serial.print(message);
    Serial.println("' but client is not connected");

    return;
  }
  
  Serial.print("> ");
  Serial.println(message);
  
  client.print(message + String("\n"));
}

