#include "mbed.h"

#include <sdram.h>

#include "Config.hpp"
#include "Application.hpp"

int main() {
	Config config;

	// configure serial
	Serial serial(config.serialTxPin, config.serialRxPin);
	serial.baud(config.serialBaudRate);

	printf("\n\n### Protocity v%s ###\n", Application::getVersion().c_str());

	// initialize sram
	if (sdram_init() != 0) {
        printf("# failed to initialize SDRAM\n");
    } else {
		printf("# initialized SDRAM\n");
	}

	// start the application
	Application application(&config, &serial);
	application.run();
}
