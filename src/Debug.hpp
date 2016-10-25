#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "mbed.h"
#include "rtos.h"

class Debug {

public:
	Debug(PinName breatheLedPin, PinName commandReceivedLedPin);

	void handleCommandReceived();

private:
	void runBreatheThread();
	void runCommandReceivedThread();

	Thread breatheThread;
	Thread commandReceivedThread;
	PwmOut breatheLed;
	DigitalOut commandReceivedLed;

	float breatheDutyCycle = 0.0f;
	int breatheDirection = 1.0f;

	const float BREATHE_PERIOD = 5.0f;
	const int BREATHE_FPS = 60;
	const int SIGNAL_COMMAND_RECEIVED = 1;
	const int LED_NOTIFICATION_DURATION_MS = 100;
};

#endif
