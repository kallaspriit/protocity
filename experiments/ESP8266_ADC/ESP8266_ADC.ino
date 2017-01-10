#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <SPI.h>

// Helpful links
// Common pins            https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/generic/common.h
// Sparkfun thing pins    https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/thing/pins_arduino.h
// Hookup guide           https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/all

//#include "MCP320X.h"

const int ADC_CS_PIN = 2;

//MCP320X adc(ADC_CS_PIN);

void setup() {
  Serial.begin(115200);
  delay(10);

  // initialize SPI and adc
  pinMode(ADC_CS_PIN, OUTPUT);
  digitalWrite(ADC_CS_PIN, HIGH);
  SPI.begin();
  //adc.begin();

  Serial.println("setup complete");
}


void loop() {
  digitalWrite(ADC_CS_PIN, LOW); 
  SPI.transfer(B01010101);
  digitalWrite(ADC_CS_PIN, HIGH);
  delay(500);

  digitalWrite(ADC_CS_PIN, LOW); 
  SPI.transfer(B10101010);
  digitalWrite(ADC_CS_PIN, HIGH);
  delay(500);

  /*
  int reading = adc.read12(adc.SINGLE_CH0);

  Serial.print("reading: ");
  Serial.print(reading);
  */
  Serial.println(".");

  delay(500);
}
