#include "TSL2561Capability.hpp"

#include "../../PortController.hpp"

TSL2561Capability::TSL2561Capability(PortController *portController) :
	AbstractCapability(portController)
{}

std::string TSL2561Capability::getName() {
	return "TSL2561";
}

CommandManager::Command::Response TSL2561Capability::execute(CommandManager::Command *command) {
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

void TSL2561Capability::enable() {
	if (isEnabled) {
		return;
	}

	printf("# enabling TSL2561 luminocity measurement every %d milliseconds\n", measurementIntervalMs);

	sensor = new TSL2561(p9, p10, TSL2561_ADDR_FLOAT);
	sensor->setGain(TSL2561_GAIN_0X);
	sensor->setTiming(TSL2561_INTEGRATIONTIME_402MS);

	timer.start();

	isEnabled = true;
}

void TSL2561Capability::disable() {
	if (!isEnabled || sensor == NULL) {
		return;
	}

	printf("# disabling TSL2561 luminocity measurement\n");

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
	int valueLux = sensor->getLuminosity(TSL2561_VISIBLE);

	snprintf(sendBuffer, SEND_BUFFER_SIZE, "%d:lux", valueLux);

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));
}
