#include "TSL2561Capability.hpp"

#include "../PortController.hpp"

TSL2561Capability::TSL2561Capability(Serial *serial, PortController *portController, PinName sdaPin, PinName sclPin) :
	AbstractCapability(serial, portController),
    sdaPin(sdaPin),
    sclPin(sclPin)
{}

std::string TSL2561Capability::getName() {
	return "TSL2561";
}

CommandManager::Command::Response TSL2561Capability::handleCommand(CommandManager::Command *command) {
	std::string action = command->getString(2);

	if (action == "enable") {
		return handleEnableCommand(command);
	} else if (action == "disable") {
		return handleDisableCommand(command);
	} else {
		return command->createFailureResponse("invalid capability action requested");
	}
}

CommandManager::Command::Response TSL2561Capability::handleEnableCommand(CommandManager::Command *command) {
	if (command->argumentCount < 3) {
        return command->createFailureResponse("no capability action requested");
    }

	// one can update the interval even if alrady enabled
	if (command->argumentCount == 4) {
		measurementIntervalMs = command->getInt(3);
	}

	if (isEnabled) {
		return command->createSuccessResponse();
	}

	enable();

	return command->createSuccessResponse();
}

CommandManager::Command::Response TSL2561Capability::handleDisableCommand(CommandManager::Command *command) {
	if (!isEnabled) {
		return command->createSuccessResponse();
	}

	disable();

	return command->createSuccessResponse();
}

void TSL2561Capability::enable() {
	if (isEnabled) {
		return;
	}

	log.info("enabling TSL2561 luminosity measurement every %d milliseconds", measurementIntervalMs);

	sensor = new TSL2561(sdaPin, sclPin, TSL2561_ADDR_FLOAT);
	sensor->setGain(TSL2561_GAIN_0X);
	sensor->setTiming(TSL2561_INTEGRATIONTIME_402MS);

	sendMeasurement();

	timer.start();

	isEnabled = true;
}

void TSL2561Capability::disable() {
	if (!isEnabled || sensor == NULL) {
		return;
	}

	log.info("disabling TSL2561 luminosity measurement");

	delete sensor;
	sensor = NULL;
	timer.stop();

	isEnabled = false;
}

void TSL2561Capability::update(int deltaUs) {
	if (!isEnabled) {
		return;
	}

	int timeSinceLastMeasurementMs = timer.read_ms();

	if (timeSinceLastMeasurementMs >= measurementIntervalMs) {
		sendMeasurement();

		timer.reset();
	}
}

void TSL2561Capability::sendMeasurement() {
	// TODO this is a blocking command, consider a thread?
	int value = sensor->getLuminosity(TSL2561_VISIBLE);

	snprintf(sendBuffer, SEND_BUFFER_SIZE, "%d:lux", value);

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));
}
