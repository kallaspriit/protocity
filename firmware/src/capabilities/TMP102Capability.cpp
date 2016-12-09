#include "TMP102Capability.hpp"

#include "../PortController.hpp"

TMP102Capability::TMP102Capability(Serial *serial, PortController *portController) :
	AbstractCapability(serial, portController)
{}

std::string TMP102Capability::getName() {
	return "TMP102";
}

CommandManager::Command::Response TMP102Capability::execute(CommandManager::Command *command) {
	std::string action = command->getString(2);

	if (action == "enable") {
		// one can update the interval even if alrady enabled
		if (command->argumentCount == 4) {
			measurementIntervalMs = command->getInt(3);
		}

		if (isEnabled) {
			return command->createSuccessResponse();
		}

		enable();

		return command->createSuccessResponse();
	} else if (action == "disable") {
		if (!isEnabled) {
			return command->createSuccessResponse();
		}

		disable();

		return command->createSuccessResponse();
	} else {
		return command->createFailureResponse("invalid capability action requested");
	}
}

void TMP102Capability::enable() {
	if (isEnabled) {
		return;
	}

	printf("# enabling TMP102 temperature measurement every %d milliseconds\n", measurementIntervalMs);

	sensor = new TMP102(p9, p10, 0x90);

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

void TMP102Capability::sendMeasurement() {
	float value = sensor->read();

	snprintf(sendBuffer, SEND_BUFFER_SIZE, "%f", value);

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));
}
