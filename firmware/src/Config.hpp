#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "mbed.h"

class Config {

public:
	// serial
	PinName serialTxPin = USBTX;
	PinName serialRxPin = USBRX;
	int serialBaudRate = 115200;

	// socket server
	int socketServerPort = 8080;

	/* lpc1768 ### */

	// digital pwm-capable ports
	PinName port1Pin = p21;
	PinName port2Pin = p22;
	PinName port3Pin = p23;
	PinName port4Pin = p24;
	PinName port5Pin = p25;

	// analog-in capable ports
	PinName port6Pin = p20;

	/* frdm_k64f ###

	// digital pwm-capable ports
	PinName port1Pin = D3;
	PinName port2Pin = D5;
	PinName port3Pin = D6;
	PinName port4Pin = D7;
	PinName port5Pin = D8;

	// analog-in capable ports
	PinName port6Pin = A0;
	*/
};

#endif
