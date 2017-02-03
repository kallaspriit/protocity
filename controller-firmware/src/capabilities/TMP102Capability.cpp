#include "TMP102Capability.hpp"

#include "../PortController.hpp"

TMP102Capability::TMP102Capability(Serial *serial, PortController *portController, PinName sdaPin, PinName sclPin) :
	AbstractCapability(serial, portController),
	sdaPin(sdaPin),
    sclPin(sclPin)
{}

std::string TMP102Capability::getName() {
	return "TMP102";
}

void TMP102Capability::update(int deltaUs) {
	if (!isEnabled) {
		return;
	}

	int timeSinceLastMeasurementMs = timer.read_ms();

	if (timeSinceLastMeasurementMs >= measurementIntervalMs) {
		sendMeasurement();

		timer.reset();
	}
}

CommandManager::Command::Response TMP102Capability::handleCommand(CommandManager::Command *command) {
	if (command->argumentCount < 3) {
        return command->createFailureResponse("no capability action requested");
    }
	
	std::string action = command->getString(2);

	if (action == "enable") {
		return handleEnableCommand(command);
	} else if (action == "disable") {
		return handleDisableCommand(command);
	} else {
		return command->createFailureResponse("invalid capability action requested");
	}
}

CommandManager::Command::Response TMP102Capability::handleEnableCommand(CommandManager::Command *command) {
	// one can update the interval even if already enabled
	if (command->argumentCount == 4) {
		measurementIntervalMs = command->getInt(3);
	}

	if (isEnabled) {
		return command->createSuccessResponse();
	}

	enable();

	return command->createSuccessResponse();
}

CommandManager::Command::Response TMP102Capability::handleDisableCommand(CommandManager::Command *command) {
	if (!isEnabled) {
		return command->createSuccessResponse();
	}

	disable();

	return command->createSuccessResponse();
}

void TMP102Capability::enable() {
	if (isEnabled) {
		return;
	}

	printf("# enabling TMP102 temperature measurement every %d milliseconds\n", measurementIntervalMs);

	sensor = new TMP102(sdaPin, sclPin, 0x90);

	sendMeasurement();

	timer.start();

	isEnabled = true;
}

void TMP102Capability::disable() {
	if (!isEnabled || sensor == NULL) {
		return;
	}

	printf("# disabling TMP102 temperature measurement\n");

	delete sensor;
	sensor = NULL;
	timer.stop();

	isEnabled = false;
}

void TMP102Capability::sendMeasurement() {
	float value = sensor->read();

	snprintf(sendBuffer, SEND_BUFFER_SIZE, "%f", value);

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));
}
