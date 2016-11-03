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
	PinName digitalPort1Pin = p21;
	PinName digitalPort2Pin = p22;
	PinName digitalPort3Pin = p23;
	PinName digitalPort4Pin = p24;
	PinName digitalPort5Pin = p25;
	PinName digitalPort6Pin = p26;
};

#endif
