#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "mbed.h"
#include "rtos.h"

#include <TLC5940.hpp>

class Application {

public:
	Application(Serial *serial);

	void run();
	void setup();
	void loop(int deltaUs);

private:
	// pin configuration
	static const PinName TLC5940_SPI_MOSI = p11;	// SPI data output
	static const PinName TLC5940_SPI_SCLK = p13;	// SPI clock
	static const PinName TLC5940_XLAT = p14;		// digital out
	static const PinName TLC5940_BLANK = p15;		// digital out
	static const PinName TLC5940_VPRG = p16;		// digital out
	static const PinName TLC5940_DCPRG = p17;		// digital out (not available on the sparkfun board)
	static const PinName TLC5940_GSCLK = p25;		// pwm out

	// breathe configuration
	const int BREATHE_UPDATE_FPS = 60;
	const float BREATHE_PERIOD = 2.0f;
	const int BREATHE_FRAME_DURATION = 1000 / BREATHE_UPDATE_FPS;
	const float BREATHE_FRAME_STEP = ((1.0f / BREATHE_PERIOD) / (float)BREATHE_UPDATE_FPS) * 2.0f;
	const int BREATHE_LED_COUNT = 16;

	// application dependencies
	DigitalOut led;
	TLC5940 ledDriver;

	// application runtime info
	int breatheDirection = 1;
	float breatheDutyCycle = 0.0f;

	// general configuration
	static const PinName SERIAL_TX = USBTX;
	static const PinName SERIAL_RX = USBRX;
	static const PinName DEBUG_LED = LED1;

	// general dependencies
	Serial *serial;
	Timer timer;

};

#endif
