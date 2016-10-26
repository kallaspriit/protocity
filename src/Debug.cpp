#include "Debug.hpp"

#include "mbed.h"

#include "Config.hpp"

#include <algorithm>

Debug::Debug() {
	ledUpdateThread.start(this, &Debug::runLedUpdateThread);
}

void Debug::setLedMode(int index, LedMode mode) {
	if (index < 0 || index > LED_COUNT - 1) {
		error("> expected led index between 0 and %d\n", LED_COUNT - 1);
	}

	ledMode[index] = mode;

	// simple states can be applied immediately
	switch (ledMode[index]) {
		case LedMode::OFF:
			ledState[index] = 0.0f;
			break;

		case LedMode::ON:
			ledState[index] = 1.0f;
			break;

		case LedMode::BLINK_ONCE:
			ledState[index] = 1.0f;
			blinkOnceState = true;
			blickOnceTimeAccumulator = 0;
			blickOncePerformed = false;

			break;

		default:
			// ignore
			break;
	}
}

void Debug::runLedUpdateThread() {
	int frameDuration = 1000 / UPDATE_FPS;
	float breatheFrameStep = ((1.0f / BREATHE_PERIOD) / (float)UPDATE_FPS) * 2.0f;
	int blinkFastHalfInterfal = FAST_BLINK_INTERVAL_MS / 2;
	int blinkSlowHalfInterfal = SLOW_BLINK_INTERVAL_MS / 2;

	while (true) {
		// update breathe state
		breatheDutyCycle = fmin(fmax(breatheDutyCycle + breatheFrameStep * (float)breatheDirection, 0.0f), 1.0f);

		if (breatheDutyCycle == 1.0f || breatheDutyCycle == 0.0f) {
			breatheDirection *= -1;
		}

		// update blink accumulators
		blinkFastTimeAccumulator += frameDuration;
		blinkSlowTimeAccumulator += frameDuration;
		blickOnceTimeAccumulator += frameDuration;

		// update blink states
		if (blinkFastTimeAccumulator > blinkFastHalfInterfal) {
			blinkFastState = !blinkFastState;

			blinkFastTimeAccumulator = 0;
		}

		if (blinkSlowTimeAccumulator > blinkSlowHalfInterfal) {
			blinkSlowState = !blinkSlowState;

			blinkSlowTimeAccumulator = 0;
		}

		if (blickOncePerformed == false && blickOnceTimeAccumulator > BLINK_ONCE_DURATION_MS) {
			blinkOnceState = false;
			blickOncePerformed = true;

			blickOnceTimeAccumulator = 0;
		}

		// apply states
		for (int i = 0; i < LED_COUNT; i++) {
			switch (ledMode[i]) {
				case LedMode::OFF:
					ledState[i] = 0.0f;
					break;

				case LedMode::ON:
					ledState[i] = 1.0f;
					break;

				case LedMode::BLINK_FAST:
					ledState[i] = blinkFastState ? 1.0f : 0.0f;
					break;

				case LedMode::BLINK_SLOW:
					ledState[i] = blinkSlowState ? 1.0f : 0.0f;
					break;

				case LedMode::BLINK_ONCE:
					ledState[i] = blinkOnceState ? 1.0f : 0.0f;
					break;

				case LedMode::BREATHE:
					ledState[i] = breatheDutyCycle;
					break;

				default:
					// ignore
					break;
			}
		}

		Thread::wait(frameDuration);
	}
}
