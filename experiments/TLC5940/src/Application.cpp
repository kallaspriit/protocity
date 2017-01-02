#include "Application.hpp"

#include <algorithm>

Application::Application(Serial *serial) :
    led(DEBUG_LED),
    ledDriver(TLC5940_SPI_MOSI, TLC5940_SPI_MISO, TLC5940_SPI_SCK, TLC5940_XLAT, TLC5940_BLANK, TLC5940_GSCLK),
 	serial(serial)
{}

void Application::run() {
    serial->printf("# setting up..\n");
	setup();
    serial->printf("# setup done!\n");

	timer.start();

    serial->printf("# starting main loop\n");
	while (true) {
		int deltaUs = timer.read_us();
		timer.reset();

		loop(deltaUs);
	}
}

void Application::setup() {
	ledDriver.run();
}

void Application::loop(int deltaUs) {
	led = !led;

    breatheDutyCycle = fmin(fmax(breatheDutyCycle + BREATHE_FRAME_STEP * (float)breatheDirection, 0.0f), 1.0f);

    // serial->printf("duty cycle: %f\n", breatheDutyCycle);

    if (breatheDutyCycle == 1.0f || breatheDutyCycle == 0.0f) {
		breatheDirection *= -1;
	}

    for (int i = 0; i < BREATHE_LED_COUNT; i++) {
        ledDriver.setValue(i, breatheDutyCycle);
    }

    ledDriver.flush();

    Thread::wait(BREATHE_FRAME_DURATION);
}
