#include "DebugCapability.hpp"

#include "../PortController.hpp"

DebugCapability::DebugCapability(Serial *serial, PortController *portController, PinName sdaPin, PinName sclPin) :
	AbstractCapability(serial, portController),
    sdaPin(sdaPin),
    sclPin(sclPin)
{}

std::string DebugCapability::getName() {
	return "debug";
}

CommandManager::Command::Response DebugCapability::handleCommand(CommandManager::Command *command) {
	if (command->argumentCount < 3) {
        return command->createFailureResponse("no capability action requested");
    }

	std::string action = command->getString(2);

	if (action == "i2c") {
		return handleI2CCommand(command);
	} else if (action == "logging") {
		return handleLoggingCommand(command);
	} else {
		return command->createFailureResponse("invalid capability action requested");
	}
}

CommandManager::Command::Response DebugCapability::handleI2CCommand(CommandManager::Command *command) {
    I2C i2c(sdaPin, sclPin);

    log.info("searching for I2C devices...");

    int count = 0;

    for (int address = 0; address < 256; address += 2) {
        if (i2c.write(address, NULL, 0) == 0) {
            log.info("- device found at address 0x%02X", address);

            count++;
        }
    }

    log.info("found %d I2C devices", count);

	return command->createSuccessResponse(count);
}

CommandManager::Command::Response DebugCapability::handleLoggingCommand(CommandManager::Command *command) {
	if (command->argumentCount < 4) {
        return command->createFailureResponse("expected a total of 4 arguments (for example '1:port:1:debug:logging:TRACE");
    }

	std::string minimumLevelInput = command->getString(3);
	Log::LogLevel minimumLevel = Log::LogHandler::parseLogLevel(minimumLevelInput.c_str());
	std::string actualMinimumLevel = Log::LogHandler::logLevelToName(minimumLevel);

	if (minimumLevelInput != actualMinimumLevel) {
		log.warn("invalid loggig level %s requested, expected one of TRACE, DEBUG, INFO, WARN, ERROR", minimumLevelInput.c_str());

		return command->createFailureResponse();
	}

	log.info("setting logging minimum level to %s", actualMinimumLevel.c_str());

	Log::getLogHandler()->setMinimumLevel(minimumLevel);

	return command->createSuccessResponse(actualMinimumLevel);
}
