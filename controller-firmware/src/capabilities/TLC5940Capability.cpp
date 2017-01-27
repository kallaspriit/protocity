#include "TLC5940Capability.hpp"

#include "../PortController.hpp"

TLC5940Capability::TLC5940Capability(Serial *serial, PortController *portController, PinName mosiPin, PinName sclkPin, PinName blankPin, PinName vprgPin, PinName gsclkPin, int chainLength) :
	AbstractCapability(serial, portController),
	mosiPin(mosiPin),
	sclkPin(sclkPin),
	blankPin(blankPin),
	vprgPin(vprgPin),
	gsclkPin(gsclkPin),
	chainLength(chainLength)
{
	values = new unsigned short[chainLength * 16];

	for (int i = 0; i < chainLength * 16; i++) {
		values[i] = 0x0000;
	}
}

std::string TLC5940Capability::getName() {
	return "TLC5940";
}

void TLC5940Capability::update(int deltaUs) {
	if (!isEnabled) {
		return;
	}

	// nothing to do periodically
}

CommandManager::Command::Response TLC5940Capability::handleCommand(CommandManager::Command *command) {
	std::string action = command->getString(2);

	if (action == "enable") {
		return handleEnableCommand(command);
	} else if (action == "disable") {
		return handleDisableCommand(command);
	} else if (action == "value") {
		return handleValueCommand(command);
	} else {
		return command->createFailureResponse("invalid capability action requested");
	}
}

CommandManager::Command::Response TLC5940Capability::handleEnableCommand(CommandManager::Command *command) {
	if (enable()) {
		return command->createSuccessResponse();
	} else {
		return command->createFailureResponse("enabling TLC5940 led driver failed");
	}
}

CommandManager::Command::Response TLC5940Capability::handleDisableCommand(CommandManager::Command *command) {
	disable();

	return command->createSuccessResponse();
}

CommandManager::Command::Response TLC5940Capability::handleValueCommand(CommandManager::Command *command) {
	if (command->argumentCount != 5) {
		return command->createFailureResponse("expected a total of 5 arguments (for example '1:port:1:TLC5940:value:0:4095')");
	}

	int channel = command->getInt(3);
	float value = command->getFloat(4);
	int maxChannel = chainLength * 16 - 1;

	if (channel < 0 || channel > maxChannel) {
		return command->createFailureResponse("invalid channel number provided");
	}

	if (value < 0.0f || value > 1.0f) {
		return command->createFailureResponse("expected a floating point value between 0..1");
	}

	if (!isEnabled) {
		printf("# setting value requested but enable not called, enabling led driver\n");

		if (!enable()) {
			return command->createFailureResponse("enabling TLC5940 led driver failed");
		}
	}

	int rawValue = min(max((int)(value * 4095.0f), 0), 4095);

	values[channel] = rawValue;

	tlc5940->setNewGSData(values);

	printf("# set channel %d to %f (%d)\n", channel, value, rawValue);

	return command->createSuccessResponse();
}

bool TLC5940Capability::enable() {
	if (isEnabled) {
		return true;
	}

	printf("# enabling TLC5940 led driver\n");

	tlc5940 = new TLC5940(sclkPin, mosiPin, gsclkPin, blankPin, portController->getPinName(), vprgPin, chainLength);

	isEnabled = true;

	return true;
}

void TLC5940Capability::disable() {
	if (!isEnabled || tlc5940 == NULL) {
		return;
	}

	printf("# disabling TLC5940 led driver\n");

	delete tlc5940;
	tlc5940 = NULL;

	isEnabled = false;
}
