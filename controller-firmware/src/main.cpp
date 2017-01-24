#include "mbed.h"

#include <sdram.h>

#include "Config.hpp"
#include "Application.hpp"

int main() {
	Config config;
	Serial serial(config.serialTxPin, config.serialRxPin);

	serial.baud(config.serialBaudRate);

	printf("\n\n### Protocity v%s ###\n", config.version.c_str());

	if (sdram_init() != 0) {
        printf("# failed to initialize SDRAM\n");
    } else {
		printf("# initialized SDRAM\n");
	}

	Application application(&config, &serial);

	application.run();
}
