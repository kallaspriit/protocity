#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "mbed.h"

#include <string>

class Config {

public:
	std::string version = "1.20.0";

	// serial
	PinName serialTxPin = USBTX;
	PinName serialRxPin = USBRX;
	int serialBaudRate = 115200;

	// socket server
	int socketServerPort = 8080;

	/* lpc1768 ### */

	// digital pwm-capable ports
	PinName port1Pin = p25;
	PinName port2Pin = p26;
	PinName port3Pin = p27;
	PinName port4Pin = p28;
	PinName port5Pin = p29;

	// analog-in capable ports
	PinName port6Pin = p15;

	// this led toggles on every main loop run
	PinName mainLoopLedPin = p33;

	// NFC SPI configuration
	PinName nfcMosiPin = p5;
	PinName nfcMisoPin = p6;
	PinName nfcSclkPin = p7;

	// Led driver configuration
	PinName ledMosiPin = p11;
	PinName ledSclkPin = p13;
	PinName ledBlankPin = p14;
	PinName ledVprgPin = p16;
	PinName ledGsclkPin = p30;
	int ledChainLength = 1;

	// i2c pin configuration
	PinName sdaPin = p9;
	PinName sclPin = p10;

	// lcd pin configuration
	PinName lcdTxPin = p37;
	PinName lcdRxPin = p31;
	PinName lcdResetPin = p8;


};

#endif
