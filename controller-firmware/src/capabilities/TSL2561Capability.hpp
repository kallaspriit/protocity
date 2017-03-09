#ifndef TSL2561CAPABILITY_HPP
#define TSL2561CAPABILITY_HPP

#include "../AbstractCapability.hpp"
#include "Log.hpp"

#include <TSL2561.hpp>

// https://www.sparkfun.com/products/12055
class TSL2561Capability : public AbstractCapability {

public:
	TSL2561Capability(Serial *serial, PortController *portController, PinName sdaPin = p9, PinName sclPin = p10);

	std::string getName();
	void update(int deltaUs);

	CommandManager::Command::Response handleCommand(CommandManager::Command *command);

private:
	Log log = Log::getLog("TSL2561Capability");

	CommandManager::Command::Response handleEnableCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDisableCommand(CommandManager::Command *command);

	void enable();
	void disable();
	void sendMeasurement();

	PinName sdaPin;
    PinName sclPin;

	Timer timer;
	TSL2561 *sensor = NULL;
	bool isEnabled = false;
	int measurementIntervalMs = 5000;
};

#endif
