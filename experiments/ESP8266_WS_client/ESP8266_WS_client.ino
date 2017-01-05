#include <ESP8266WiFi.h>
#include <WebSocketClient.h>

const char* ssid     = "Stagnationlab";
const char* password = "purgisupp";

/*
const char* ssid     = "Priit";
const char* password = "purgisupp";
*/

/*
char host[] = "echo.websocket.org";
int port = 80;
char path[] = "/";
*/

char host[] = "10.220.20.140";
int port = 8080;
char path[] = "/";

WebSocketClient webSocketClient;

// Use WiFiClient class to create TCP connections
WiFiClient client;

void setup() {
  Serial.begin(9600);
  delay(10);

  // setup pins
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    toggleLed();
  }

  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(5000);

  Serial.print("Connecting to ws://");
  Serial.print(host);
  Serial.print(":");
  Serial.print(port);
  Serial.println(path);

  // Connect to the websocket server
  //if (client.connect("echo.websocket.org", 80)) {
  if (client.connect(host, port)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    while(1) {
      // Hang on failure
    }
  }

  Serial.println("Performing handshake");

  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    Serial.println("Handshake failed.");
    while(1) {
      // Hang on failure
    }  
  }

  Serial.println("Setup complete!");
}


void loop() {
  String data;

  if (client.connected()) {
    webSocketClient.getData(data);
    
    if (data.length() > 0) {
      Serial.print("< ");
      Serial.println(data);

      if (data == "toggle-led") {
        toggleLed();

        data = "led:" + String(digitalRead(BUILTIN_LED));
        webSocketClient.sendData(data);
      }
    }

    /*
    data = "Hey!";

    Serial.print("> ");
    Serial.println(data);
    
    webSocketClient.sendData(data);
    */
    
  } else {
    Serial.println("Client disconnected.");
    while (1) {
      // Hang on disconnect.
    }
  }
  
  // wait to fully let the client disconnect
  //delay(3000);
  
}

void toggleLed() {
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH);
}

