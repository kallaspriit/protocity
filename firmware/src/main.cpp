#include "mbed.h"

#include <sdram.h>

#include "Config.hpp"
#include "Application.hpp"

int main() {
	Config config;
	Serial serial(config.serialTxPin, config.serialRxPin);

	serial.baud(config.serialBaudRate);

	printf("### PrototypeCity ###\n");

	if (sdram_init() != 0) {
        printf("# failed to initialize SDRAM\n");
    } else {
		printf("# SDRAM is initialized\n");
	}

	Application application(&config, &serial);

	application.run();
}
