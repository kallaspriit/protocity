#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "mbed.h"

class Config {

public:
	// serial
	PinName serialTxPin = USBTX;
	PinName serialRxPin = USBRX;
	int serialBaudRate = 115200;

	// debug
	PinName debugBreatheLedPin = LED1;
	PinName debugCommandReceivedLedPin = LED2;

};

#endif
