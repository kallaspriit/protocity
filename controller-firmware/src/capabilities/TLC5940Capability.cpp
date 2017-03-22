#include "TLC5940Capability.hpp"

#include "../PortController.hpp"
#include "../Util.hpp"

TLC5940Capability::TLC5940Capability(Serial *serial, PortController *portController, PinName mosiPin, PinName sclkPin, PinName blankPin, PinName errorPin, PinName gsclkPin, int chainLength) :
	AbstractCapability(serial, portController),
	mosiPin(mosiPin),
	sclkPin(sclkPin),
	blankPin(blankPin),
	errorPin(errorPin),
	gsclkPin(gsclkPin),
	chainLength(chainLength)
{
	values = new unsigned short[chainLength * CHANNEL_COUNT];
}

std::string TLC5940Capability::getName() {
	return "TLC5940";
}

void TLC5940Capability::update(int deltaUs) {
	if (!isEnabled) {
		return;
	}

	/*
	timeSinceLastRefreshUs += deltaUs;

	// periodically refresh the output values
	if (timeSinceLastRefreshUs >= REFRESH_INTERVAL_US) {
		// log.trace("refreshing output values");

		sendData();

		timeSinceLastRefreshUs = 0;
	}
	*/
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
		log.info("setting value requested but enable not called, enabling led driver");

		if (!enable()) {
			return command->createFailureResponse("enabling TLC5940 led driver failed");
		}
	}

	int channel = command->getInt(3);
	float value = command->getFloat(4);

	if (!setChannelValue(channel, value)) {
		return command->createFailureResponse("setting requested value failed, check parameters");
	}

	sendData();

	if (tlc5940->isErrorDetected()) {
		return command->createFailureResponse("output driver error condition detected");
	} else {
		return command->createSuccessResponse();
	}
}

CommandManager::Command::Response TLC5940Capability::handleValuesCommand(CommandManager::Command *command) {
	if (command->argumentCount != 4) {
		return command->createFailureResponse("expected a total of 4 arguments (for example '1:port:1:TLC5940:values:0-0.5,1-1.0,3-0.0')");
	}

	if (!isEnabled) {
		log.info("setting value requested but enable not called, enabling led driver");

		if (!enable()) {
			return command->createFailureResponse("enabling TLC5940 led driver failed");
		}
	}

	std::string values = command->getString(3);

	log.trace("requested to set values: %s", values.c_str());

	std::vector<std::string> channelValuePairs = Util::split(values, ',');

	for (std::vector<std::string>::iterator it = channelValuePairs.begin(); it != channelValuePairs.end(); it++) {
		std::string channelValuePair = *it;

		std::vector<std::string> channelValueTokens = Util::split(channelValuePair, '-');

		if (channelValueTokens.size() != 2) {
			return command->createFailureResponse("invalid values requested");
		}

		int channel = atoi(channelValueTokens.at(0).c_str());
		float value = atof(channelValueTokens.at(1).c_str());

		log.trace(" setting channel %d to %f", channel, value);

		if (!setChannelValue(channel, value)) {
			return command->createFailureResponse("setting requested values failed, check parameters");
		}
	}

	// manually send the data in the end
	sendData();

	if (tlc5940->isErrorDetected()) {
		return command->createFailureResponse("output driver error condition detected");
	} else {
		return command->createSuccessResponse();
	}
}

CommandManager::Command::Response TLC5940Capability::handleTestCommand(CommandManager::Command *command) {
	bool wasEnabled = isEnabled;

	if (!isEnabled) {
		log.info("test requested but enable not called, enabling led driver");

		if (!enable()) {
			return command->createFailureResponse("enabling TLC5940 led driver failed");
		}
	}

	int channelCount = chainLength * CHANNEL_COUNT;
	int step = 2;
	int times = 1;

	if (command->argumentCount >= 4) {
		times = command->getInt(3);
	}

	// fade all channels in and out, blocks the thread!
	for (int i = 0; i < times; i++) {
		for (int j = 0; j <= 200; j += step) {
			float value = (float)(j < 100 ? j : 200 - j) / 100.0f;

			for (int channel = 0; channel < channelCount; channel++) {
				setChannelValue(channel, value);
			}

			sendData();

			Thread::wait(20); // < 50 FPS;
		}
	}

	if (!wasEnabled) {
		disable();
	}

	if (tlc5940->isErrorDetected()) {
		return command->createFailureResponse("output driver error condition detected");
	} else {
		return command->createSuccessResponse();
	}
}

bool TLC5940Capability::enable() {
	if (isEnabled) {
		return true;
	}

	log.info("enabling TLC5940 led driver");

	tlc5940 = new TLC5940(sclkPin, mosiPin, gsclkPin, blankPin, portController->getPinName(), errorPin, chainLength);

	if (tlc5940->isErrorDetected()) {
		log.warn("led driver is indicating error already before sending data to it, this should not happen");

		return false;
	}

	isEnabled = true;

	reset();

	// wait a bit for a possible error condition detection
	Thread::wait(50);

	if (tlc5940->isErrorDetected()) {
		isEnabled = false;

		return false;
	}

	isEnabled = true;

	return true;
}

void TLC5940Capability::disable() {
	if (!isEnabled || tlc5940 == NULL) {
		return;
	}

	log.info("disabling TLC5940 led driver");

	reset();

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
		log.warn("invalid channel index %d requested (expected between 0 and %d)", channel, maxChannel);

		return false;
	}

	if (value < 0.0f || value > 1.0f) {
		log.warn("invalid channel value %f requested (expected between 0.0 and 1.0)", value);

		return false;
	}

	int rawValue = min(max((int)(value * 4095.0f), 0), 4095);

	values[channel] = rawValue;

	// log.trace("set channel %d: %f (raw: %d)", channel, value, rawValue);

	return true;
}

void TLC5940Capability::reset() {
	log.debug("resetting");

	for (int i = 0; i < chainLength * CHANNEL_COUNT; i++) {
		values[i] = 0;
	}

	if (isEnabled) {
		sendData();
	}
}

void TLC5940Capability::sendData() {
	if (!isEnabled) {
		log.warn("sending data requested but not enabled");

		return;
	}

	log.trace("sending data");

	tlc5940->setNewGSData(values);
}
