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
};

#endif
