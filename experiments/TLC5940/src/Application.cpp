#include "Application.hpp"

#include <algorithm>

Application::Application(Serial *serial) :
    led(DEBUG_LED),
    ledDriver(TLC5940_SPI_SCLK, TLC5940_SPI_MOSI, TLC5940_GSCLK, TLC5940_BLANK, TLC5940_XLAT, TLC5940_DCPRG, TLC5940_VPRG, 1),
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
	// ledDriver.run();
}

void Application::loop(int deltaUs) {
	led = !led;

    breatheDutyCycle = fmin(fmax(breatheDutyCycle + BREATHE_FRAME_STEP * (float)breatheDirection, 0.0f), 1.0f);

    if (breatheDutyCycle == 1.0f || breatheDutyCycle == 0.0f) {
		breatheDirection *= -1;
	}

    for (int i = 0; i < BREATHE_LED_COUNT; i++) {
        //ledDriver.setValue(i, breatheDutyCycle);
    }

    // Create a buffer to store the data to be sent
    unsigned short GSData[16] = { 0x0000 };
    unsigned short value = (unsigned short)(breatheDutyCycle * 4095.0f);

    serial->printf("power: %f - %d\n", breatheDutyCycle, value);

    for (int i = 0; i < BREATHE_LED_COUNT; i++) {
        //ledDriver.setValue(i, breatheDutyCycle);

        GSData[i] = value;
    }

    // Enable the first LED
    // GSData[0] = 0xFFF;

    ledDriver.setNewGSData(GSData);

    // ledDriver.flush();

    Thread::wait(BREATHE_FRAME_DURATION);
}
