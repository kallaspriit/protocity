#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "Log.hpp"

#include "mbed.h"
#include "rtos.h"

class Debug {

public:
	enum LedMode {
		OFF,
		ON,
		BLINK_SLOW,
		BLINK_FAST,
		BLINK_ONCE,
		BREATHE
	};

	Debug();

	void setLedMode(int index, LedMode mode);
	static int getTotalMemoryBytes();
	static int getFreeMemoryBytes();

private:
	Log log = Log::getLog("Debug");

	void runLedUpdateThread();

	Thread ledUpdateThread;

	float breatheDutyCycle = 0.0f;
	int breatheDirection = 1.0f;
	int blinkFastTimeAccumulator = 0;
	int blinkSlowTimeAccumulator = 0;
	int blickOnceTimeAccumulator = 0;
	bool blinkFastState = false;
	bool blinkSlowState = false;
	bool blinkOnceState = false;
	bool blickOncePerformed = true;

	static const int LED_COUNT = 4;
	static const int FREE_MEMORY_CELL_SIZE = 100 * 1000; // 100kb accuracy
	const float BREATHE_PERIOD = 2.0f;
	const int UPDATE_FPS = 60;
	const int FAST_BLINK_INTERVAL_MS = 200;
	const int SLOW_BLINK_INTERVAL_MS = 2000;
	const int BLINK_ONCE_DURATION_MS = 100;

	LedMode ledMode[LED_COUNT] = { LedMode::OFF };
	// PwmOut ledState[LED_COUNT] = { LED1, LED2, LED3, LED4 };
	DigitalOut ledState[LED_COUNT] = { LED1, LED2, LED3, LED4 };

	struct FreeMemoryTestElement {
	    struct FreeMemoryTestElement *next;
	    char dummy[FREE_MEMORY_CELL_SIZE - 2];
	};
};

#endif
