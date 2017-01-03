#define ESP8266_LED 5

void setup() 
{
  pinMode(ESP8266_LED, OUTPUT);
}

void loop() 
{
  digitalWrite(ESP8266_LED, HIGH);
  delay(100);
  digitalWrite(ESP8266_LED, LOW);
  delay(100);
}
