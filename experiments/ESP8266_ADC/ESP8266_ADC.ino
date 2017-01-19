#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <SPI.h>

// Helpful links
// Common pins            https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/generic/common.h
// Sparkfun thing pins    https://github.com/esp8266/Arduino/blob/3e7b4b8e0cf4e1f7ad48104abfc42723b5e4f9be/variants/thing/pins_arduino.h
// Hookup guide           https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/all

#include "MCP320X.h"

const int ADC_CS_PIN = 2;

MCP320X adc(ADC_CS_PIN);

void setup() {
  Serial.begin(115200);
  delay(10);

  // initialize SPI and adc
  pinMode(ADC_CS_PIN, OUTPUT);
  digitalWrite(ADC_CS_PIN, HIGH);
  SPI.begin();
  adc.begin();

  Serial.println("setup complete");
}


void loop() {
  /*
  digitalWrite(ADC_CS_PIN, LOW); 
  SPI.transfer(B01010101);
  digitalWrite(ADC_CS_PIN, HIGH);
  delay(500);

  digitalWrite(ADC_CS_PIN, LOW); 
  SPI.transfer(B10101010);
  digitalWrite(ADC_CS_PIN, HIGH);
  delay(500);
  */

  // configuration
  float resistor1 = 8200.0; // between input and output
  float resistor2 = 15000.0f; // between input and ground
  float calibrationMultiplier = 0.99f; // multimeter-measured voltage / reported voltage
  int maxReading = 4095;
  float maxReadingVoltage = 3.3f; // Vcc/Vref pinpin

  // calculation
  int voltageReading = adc.read12(adc.SINGLE_CH0);
  int distanceReading = adc.read12(adc.SINGLE_CH1);

  float actualVoltage = calculateAdcVoltage(voltageReading, maxReading, maxReadingVoltage, resistor1, resistor2, calibrationMultiplier);
  float distanceVoltage = calculateAdcVoltage(distanceReading, maxReading, maxReadingVoltage, 0, 1, calibrationMultiplier);
  float distance = max(min(13.0f * pow(distanceVoltage, -1), 30.0f), 4.0f);

  bool isCharging = actualVoltage > 4.3f;

  Serial.print("voltage reading: ");
  Serial.print(voltageReading);
  Serial.print(", actual voltage: ");
  Serial.print(actualVoltage);
  Serial.print("V, is charging: ");
  Serial.print(isCharging ? "yes" : "no");
  
  Serial.print(", distance reading: ");
  Serial.print(distanceReading);
  Serial.print(", distance voltage: ");
  Serial.print(distanceVoltage);
  Serial.print("V, distance: ");
  Serial.print(distance);
  Serial.print("cm");
  
  Serial.println(".");

  delay(500);
}

float calculateAdcVoltage(int reading, int maxReading, float maxReadingVoltage, float resistor1, float resistor2, float calibrationMultiplier) {
  float sensedVoltage = ((float)reading / (float)maxReading) * maxReadingVoltage * calibrationMultiplier;
  float actualVoltage = sensedVoltage / (resistor2 / (resistor1 + resistor2));

  return actualVoltage;
}

