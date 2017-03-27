#ifndef TLC5940CAPABILITY_HPP
#define TLC5940CAPABILITY_HPP

#include "../AbstractCapability.hpp"
#include "Log.hpp"

#include <TLC5940.hpp>

#include <string>

class TLC5940Capability : public AbstractCapability {

public:
	TLC5940Capability(Serial *serial, PortController *portController, PinName mosiPin, PinName sclkPin, PinName blankPin, PinName errorPin, PinName gsclkPin, int chainLength = 1);

	std::string getName();
	void update(int deltaUs);

	CommandManager::Command::Response handleCommand(CommandManager::Command *command);

private:
	Log log = Log::getLog("TLC5940Capability");

	CommandManager::Command::Response handleEnableCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDisableCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleValueCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleValuesCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleTestCommand(CommandManager::Command *command);

	bool enable();
	void disable();
	bool setChannelValue(int channel, float value);
	void reset();
	void sendData();

	static const int CHANNEL_COUNT = 16;
	static const int REFRESH_INTERVAL_US = 1 * 1000 * 1000;

	const PinName mosiPin;
	const PinName sclkPin;
	const PinName blankPin;
	const PinName errorPin;
	const PinName gsclkPin;
	const int chainLength;

	TLC5940 *tlc5940 = NULL;
	bool isEnabled = false;
	std::string activeTagName = "";
	int timeSinceLastRefreshUs = 0;

	unsigned short *values;
};

#endif
