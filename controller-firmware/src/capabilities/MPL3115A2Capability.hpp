#ifndef MPL3115A2CAPABILITY_HPP
#define MPL3115A2CAPABILITY_HPP

#include "../AbstractCapability.hpp"

#include <MPL3115A2.hpp>

// https://www.sparkfun.com/products/11084
class MPL3115A2Capability : public AbstractCapability {

public:
	MPL3115A2Capability(Serial *serial, PortController *portController, PinName sdaPin = p9, PinName sclPin = p10);

	std::string getName();
	void update(int deltaUs);

	CommandManager::Command::Response handleCommand(CommandManager::Command *command);

private:
	Log log = Log::getLog("MPL3115A2Capability");

	CommandManager::Command::Response handleEnableCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDisableCommand(CommandManager::Command *command);

	void enable();
	void disable();
	void sendMeasurement();

	PinName sdaPin;
    PinName sclPin;

	Timer timer;
	MPL3115A2 *sensor = NULL;
	bool isEnabled = false;
	int measurementIntervalMs = 5000;
};

#endif
