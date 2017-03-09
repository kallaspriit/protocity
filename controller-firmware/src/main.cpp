#include "mbed.h"

#include <sdram.h>

#include "Config.hpp"
#include "Log.hpp"
#include "Application.hpp"

int main() {
	Config config;
	Log log = Log::getLog("main");

	// configure serial
	Serial serial(config.serialTxPin, config.serialRxPin);
	serial.baud(config.serialBaudRate);

	log.info("starting Protocity v%s", Application::getVersion().c_str());

	// initialize sram
	if (sdram_init() != 0) {
        log.error("failed to initialize SDRAM");
    } else {
		log.info("initialized SDRAM");
	}

	// start the application
	Application application(&config, &serial);
	application.run();

	log.info("application completed");
}
