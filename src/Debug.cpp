#include "Debug.hpp"

#include "Config.hpp"

#include <algorithm>

Debug::Debug(PinName breatheLedPin, PinName commandReceivedLedPin) :
 	breatheLed(breatheLedPin),
	commandReceivedLed(commandReceivedLedPin)
{
	breatheThread.start(this, &Debug::runBreatheThread);
	commandReceivedThread.start(this, &Debug::runCommandReceivedThread);
}

void Debug::runBreatheThread() {
	int frameDuration = 1000 / BREATHE_FPS;
	float frameStep = ((1.0f / BREATHE_PERIOD) / (float)BREATHE_FPS) * 2.0f;

	while (true) {
		breatheDutyCycle = fmin(fmax(breatheDutyCycle + frameStep * (float)breatheDirection, 0.0f), 1.0f);

		if (breatheDutyCycle == 1.0f || breatheDutyCycle == 0.0f) {
			breatheDirection *= -1;
		}

		breatheLed = breatheDutyCycle;

		Thread::wait(frameDuration);
	}
}

void Debug::runCommandReceivedThread() {
	while (true) {
		Thread::signal_wait(SIGNAL_COMMAND_RECEIVED);

		commandReceivedLed = 1;
		Thread::wait(LED_NOTIFICATION_DURATION_MS);
		commandReceivedLed = 0;
	}
}

void Debug::handleCommandReceived() {
	commandReceivedThread.signal_set(SIGNAL_COMMAND_RECEIVED);
}
