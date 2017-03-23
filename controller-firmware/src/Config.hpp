#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "mbed.h"

#include <string>

class Config {

public:
	// serial
	PinName serialTxPin = USBTX;
	PinName serialRxPin = USBRX;
	int serialBaudRate = 115200;

	// socket server
	int socketServerPort = 8080;

	// interrupt pins: p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p23,p33,p34

	// pins
	PinName port1Pin = p25; // digital, pwm
	PinName port2Pin = p26; // digital, pwm
	PinName port3Pin = p27; // digital, pwm
	PinName port4Pin = p28; // digital, pwm
	PinName port5Pin = p29; // digital, pwm
	PinName port6Pin = p15; // digital, analog in
	PinName port7Pin = p34; // digital, interrupt

	// this led toggles on every main loop run
	PinName mainLoopLedPin = p33;

	// NFC SPI configuration (SPI0)
	PinName nfcMosiPin = p5;
	PinName nfcMisoPin = p6;
	PinName nfcSclkPin = p7;

	// Led driver configuration (SPI1)
	PinName ledMosiPin = p11;
	PinName ledSclkPin = p13;
	PinName ledBlankPin = p14;
	PinName ledErrorPin = p16;
	PinName ledGsclkPin = p30;
	int ledChainLength = 1;

	// i2c pin configuration (I2C1)
	PinName sdaPin = p9;
	PinName sclPin = p10;

	// lcd pin configuration (UART4)
	PinName lcdTxPin = p37;
	PinName lcdRxPin = p31;
	PinName lcdResetPin = p8;

};

#endif
