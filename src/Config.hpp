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

	// ports
	PinName port1Pin = p21;
	PinName port2Pin = p22;
	PinName port3Pin = p23;
	PinName port4Pin = p24;
	PinName port5Pin = p25;
	PinName port6Pin = p26;
};

#endif
