#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "mbed.h"
#include "rtos.h"

class Debug {

public:
	Debug();

	void runBreatheThread();

private:
	Thread breatheThread;
	PwmOut breatheLed;

	float breatheDutyCycle = 0.0f;
	int breatheDirection = 1.0f;

	const float BREATHE_PERIOD = 5.0f;
	const int BREATHE_FPS = 60;
};

#endif
