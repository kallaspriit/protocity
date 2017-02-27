#include "TLC5940Capability.hpp"

#include "../PortController.hpp"
#include "../Util.hpp"

TLC5940Capability::TLC5940Capability(Serial *serial, PortController *portController, PinName mosiPin, PinName sclkPin, PinName blankPin, PinName vprgPin, PinName gsclkPin, int chainLength) :
	AbstractCapability(serial, portController),
	mosiPin(mosiPin),
	sclkPin(sclkPin),
	blankPin(blankPin),
	vprgPin(vprgPin),
	gsclkPin(gsclkPin),
	chainLength(chainLength)
{
	values = new unsigned short[chainLength * CHANNEL_COUNT];

	for (int i = 0; i < chainLength * CHANNEL_COUNT; i++) {
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
	if (command->argumentCount < 3) {
        return command->createFailureResponse("no capability action requested");
    }

	std::string action = command->getString(2);

	if (action == "enable") {
		return handleEnableCommand(command);
	} else if (action == "disable") {
		return handleDisableCommand(command);
	} else if (action == "value") {
		return handleValueCommand(command);
	} else if (action == "values") {
		return handleValuesCommand(command);
	} else if (action == "test") {
		return handleTestCommand(command);
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
		return command->createFailureResponse("expected a total of 5 arguments (for example '1:port:1:TLC5940:value:0:0.5')");
	}

	if (!isEnabled) {
		printf("# setting value requested but enable not called, enabling led driver\n");

		if (!enable()) {
			return command->createFailureResponse("enabling TLC5940 led driver failed");
		}
	}

	int channel = command->getInt(3);
	float value = command->getFloat(4);

	if (!setChannelValue(channel, value)) {
		return command->createFailureResponse("setting requested value failed, check parameters");
	}

	printf("# set channel %d to %f\n", channel, value);

	return command->createSuccessResponse();
}

CommandManager::Command::Response TLC5940Capability::handleValuesCommand(CommandManager::Command *command) {
	if (command->argumentCount != 4) {
		return command->createFailureResponse("expected a total of 4 arguments (for example '1:port:1:TLC5940:values:0-0.5,1-1.0,3-0.0')");
	}

	if (!isEnabled) {
		printf("# setting value requested but enable not called, enabling led driver\n");

		if (!enable()) {
			return command->createFailureResponse("enabling TLC5940 led driver failed");
		}
	}

	std::string values = command->getString(3);

	printf("# requested to set values: %s\n", values.c_str());

	std::vector<std::string> channelValuePairs = Util::split(values, ',');

	for (std::vector<std::string>::iterator it = channelValuePairs.begin(); it != channelValuePairs.end(); it++) {
		std::string channelValuePair = *it;

		std::vector<std::string> channelValueTokens = Util::split(channelValuePair, '-');

		if (channelValueTokens.size() != 2) {
			return command->createFailureResponse("invalid values requested");
		}

		int channel = atoi(channelValueTokens.at(0).c_str());
		float value = atof(channelValueTokens.at(1).c_str());

		printf("#  setting channel %d to %f\n", channel, value);

		if (!setChannelValue(channel, value)) {
			return command->createFailureResponse("setting requested values failed, check parameters");
		}
	}

	return command->createSuccessResponse();
}

CommandManager::Command::Response TLC5940Capability::handleTestCommand(CommandManager::Command *command) {
	if (!isEnabled) {
		printf("# test requested but enable not called, enabling led driver\n");

		if (!enable()) {
			return command->createFailureResponse("enabling TLC5940 led driver failed");
		}
	}

	int channelCount = chainLength * CHANNEL_COUNT;

	// fade all channels in and out once, blocks the thread!
	for (int i = 0; i <= 200; i++) {
		float value = (float)(i < 100 ? i : 200 - i) / 100.0f;

		for (int channel = 0; channel < channelCount; channel++) {
			setChannelValue(channel, value);
		}

		Thread::wait(10);
	}

	return command->createSuccessResponse();
}

bool TLC5940Capability::enable() {
	if (isEnabled) {
		return true;
	}

	printf("# enabling TLC5940 led driver\n");

	tlc5940 = new TLC5940(sclkPin, mosiPin, gsclkPin, blankPin, portController->getPinName(), vprgPin, chainLength);

	isEnabled = true;

	int channelCount = chainLength * CHANNEL_COUNT;

	// initialize all channels to off state
	for (int channel = 0; channel < channelCount; channel++) {
		setChannelValue(channel, 0.0f);
	}

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

bool TLC5940Capability::setChannelValue(int channel, float value) {
	if (!isEnabled || tlc5940 == NULL) {
		return false;
	}

	int maxChannel = chainLength * CHANNEL_COUNT - 1;

	if (channel < 0 || channel > maxChannel) {
		return false;
	}

	if (value < 0.0f || value > 1.0f) {
		return false;
	}

	int rawValue = min(max((int)(value * 4095.0f), 0), 4095);

	values[channel] = rawValue;

	tlc5940->setNewGSData(values);

	return true;
}
