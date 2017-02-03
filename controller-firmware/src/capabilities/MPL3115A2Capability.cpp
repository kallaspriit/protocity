#include "MPL3115A2Capability.hpp"

#include "../PortController.hpp"

MPL3115A2Capability::MPL3115A2Capability(Serial *serial, PortController *portController, PinName sdaPin, PinName sclPin) :
	AbstractCapability(serial, portController),
	sdaPin(sdaPin),
    sclPin(sclPin)
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

	printf("# enabling MPL3115A2 pressure measurement every %d milliseconds\n", measurementIntervalMs);

	sensor = new MPL3115A2(sdaPin, sclPin, 0x60 << 1);

	sensor->Oversample_Ratio(OVERSAMPLE_RATIO_32);
	sensor->Barometric_Mode();

	sendMeasurement();

	timer.start();

	isEnabled = true;
}

void MPL3115A2Capability::disable() {
	if (!isEnabled || sensor == NULL) {
		return;
	}

	printf("# disabling MPL3115A2 pressure measurement\n");

	delete sensor;
	sensor = NULL;
	timer.stop();

	isEnabled = false;
}

void MPL3115A2Capability::sendMeasurement() {
	float pressureKpa = sensor->getPressure() / 1000.0f;
	float pressureMmHg = pressureKpa / 0.13332239f;
	float temperature = sensor->getTemperature();

	snprintf(sendBuffer, SEND_BUFFER_SIZE, "%f:%f:%f", pressureKpa, pressureMmHg, temperature);

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));
}
