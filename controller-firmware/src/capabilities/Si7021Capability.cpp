#include "Si7021Capability.hpp"

#include "../PortController.hpp"

Si7021Capability::Si7021Capability(Serial *serial, PortController *portController, PinName sdaPin, PinName sclPin) :
	AbstractCapability(serial, portController),
    sdaPin(sdaPin),
    sclPin(sclPin)
{}

std::string Si7021Capability::getName() {
	return "Si7021";
}

void Si7021Capability::update(int deltaUs) {
	if (!isEnabled) {
		return;
	}

	int timeSinceLastMeasurementMs = timer.read_ms();

	if (timeSinceLastMeasurementMs >= measurementIntervalMs) {
		sendMeasurement();

		timer.reset();
	}
}

CommandManager::Command::Response Si7021Capability::handleCommand(CommandManager::Command *command) {
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

CommandManager::Command::Response Si7021Capability::handleEnableCommand(CommandManager::Command *command) {
	// one can update the interval even if already enabled
	if (command->argumentCount == 4) {
		measurementIntervalMs = command->getInt(3);
	}

	if (isEnabled) {
		return command->createSuccessResponse();
	}

	if (!enable()) {
		return command->createFailureResponse("enabling the sensor failed");
	}

	return command->createSuccessResponse();
}

CommandManager::Command::Response Si7021Capability::handleDisableCommand(CommandManager::Command *command) {
	if (!isEnabled) {
		return command->createSuccessResponse();
	}

	disable();

	return command->createSuccessResponse();
}

bool Si7021Capability::enable() {
	if (isEnabled) {
		return true;
	}

	log.info("enabling Si7021 humidity measurement every %d milliseconds", measurementIntervalMs);

	sensor = new Si7021(sdaPin, sclPin);

	if (!sensor->check()) {
        log.warn("Si7021 humidity sensor check failed");

        return false;
    }

	sendMeasurement();

	timer.start();

	isEnabled = true;

	return true;
}

void Si7021Capability::disable() {
	if (!isEnabled || sensor == NULL) {
		return;
	}

	log.info("disabling Si7021 humidity measurement");

	delete sensor;
	sensor = NULL;
	timer.stop();

	isEnabled = false;
}

void Si7021Capability::sendMeasurement() {
    bool wasMeasurementSuccessful = sensor->measure();

    if (!wasMeasurementSuccessful) {
        log.warn("measuring humidity failed");

        return;
    }

	float value = (float)sensor->get_humidity() / 1000.0f;

	snprintf(sendBuffer, SEND_BUFFER_SIZE, "%f", value);

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));
}
