#ifndef TMP102CAPABILITY_HPP
#define TMP102CAPABILITY_HPP

#include "../AbstractCapability.hpp"

#include <TMP102.hpp>

// https://www.sparkfun.com/products/11931
class TMP102Capability : public AbstractCapability {

public:
	TMP102Capability(Serial *serial, PortController *portController, PinName sdaPin = p9, PinName sclPin = p10);

	std::string getName();
	void update(int deltaUs);

	CommandManager::Command::Response handleCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleEnableCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDisableCommand(CommandManager::Command *command);

private:
	void enable();
	void disable();
	void sendMeasurement();

	PinName sdaPin;
    PinName sclPin;

	Timer timer;
	TMP102 *sensor = NULL;
	bool isEnabled = false;
	int measurementIntervalMs = 5000;
};

#endif
