#include "mbed.h"
#include "rtos.h"

#include <sdram.h>

#include "Config.hpp"
#include "Log.hpp"
#include "Application.hpp"

class SerialLogHandler : public Log::LogHandler {

public:
	SerialLogHandler(Log::LogLevel minimumLevel) : LogHandler(minimumLevel) {}

	void handleLogMessage(Log::LogLevel level, const char *component, const char *message) {
		if (level < minimumLevel) {
			return;
		}

		serialMutex.lock();

		printf("# %-5s | %-35s | %s\n", logLevelToName(level), component, message);

		serialMutex.unlock();
	};

private:
	Mutex serialMutex;
};

int main() {
	// set log handler to use along with the minimum level of detail to show
	Log::setLogHandler(new SerialLogHandler(Log::LogLevel::DEBUG));

	Config config;
	Log log = Log::getLog("main");

	// configure serial
	Serial serial(config.serialTxPin, config.serialRxPin);
	serial.baud(config.serialBaudRate);
	printf("\n\n");

	log.info("starting Protocity v%s", Application::getVersion().c_str());
	log.info("logging level can be changed with command '1:port:1:debug:logging:TRACE' etc (where level is one of TRACE, DEBUG, INFO, WARN, ERROR)");

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
