#include "MPL3115A2Capability.hpp"

#include "../PortController.hpp"

MPL3115A2Capability::MPL3115A2Capability(Serial *serial, PortController *portController) :
	AbstractCapability(serial, portController)
{}

std::string MPL3115A2Capability::getName() {
	return "MPL3115A2";
}

void MPL3115A2Capability::update(int deltaUs) {
	if (!isEnabled) {
		return;
	}

	int timeSinceLastMeasurementMs = timer.read_ms();

	if (timeSinceLastMeasurementMs >= measurementIntervalMs) {
		sendMeasurement();

		timer.reset();
	}
}

CommandManager::Command::Response MPL3115A2Capability::handleCommand(CommandManager::Command *command) {
	std::string action = command->getString(2);

	if (action == "enable") {
		return handleEnableCommand(command);
	} else if (action == "disable") {
		return handleDisableCommand(command);
	} else {
		return command->createFailureResponse("invalid capability action requested");
	}
}

CommandManager::Command::Response MPL3115A2Capability::handleEnableCommand(CommandManager::Command *command) {
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

CommandManager::Command::Response MPL3115A2Capability::handleDisableCommand(CommandManager::Command *command) {
	if (!isEnabled) {
		return command->createSuccessResponse();
	}

	disable();

	return command->createSuccessResponse();
}

void MPL3115A2Capability::enable() {
	if (isEnabled) {
		return;
	}

	printf("# enabling TMP102 temperature measurement every %d milliseconds\n", measurementIntervalMs);

	sensor = new MPL3115A2(p9, p10, 0x60 << 1);

	sensor->Oversample_Ratio( OVERSAMPLE_RATIO_32);
	sensor->Altimeter_Mode();

	timer.start();

	isEnabled = true;
}

void MPL3115A2Capability::disable() {
	if (!isEnabled || sensor == NULL) {
		return;
	}

	printf("# disabling TMP102 temperature measurement\n");

	delete sensor;
	sensor = NULL;
	timer.stop();

	isEnabled = false;
}

void MPL3115A2Capability::sendMeasurement() {
	float values[2];

	sensor->getAllData(&values[0]);

	snprintf(sendBuffer, SEND_BUFFER_SIZE, "%f:%f", values[0], values[1]);

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));
}
