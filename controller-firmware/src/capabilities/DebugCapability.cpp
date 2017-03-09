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
