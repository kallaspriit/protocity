#include "Debug.hpp"

#include <algorithm>

Debug::Debug() :
 	breatheLed(LED1)
{
	breatheThread.start(this, &Debug::runBreatheThread);
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

		// printf("> LED: %f\n", breatheDutyCycle);

		Thread::wait(frameDuration);
	}
}
