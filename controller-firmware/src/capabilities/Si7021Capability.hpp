#ifndef SI7021CAPABILITY_HPP
#define SI7021CAPABILITY_HPP

#include "../AbstractCapability.hpp"
#include "Log.hpp"

#include <Si7021.hpp>

class Si7021Capability : public AbstractCapability {

public:
	Si7021Capability(Serial *serial, PortController *portController, PinName sdaPin = p9, PinName sclPin = p10);

	std::string getName();
	void update(int deltaUs);

	CommandManager::Command::Response handleCommand(CommandManager::Command *command);

private:
	Log log = Log::getLog("Si7021Capability");

	CommandManager::Command::Response handleEnableCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDisableCommand(CommandManager::Command *command);

	bool enable();
	void disable();
    void sendMeasurement();

    PinName sdaPin;
    PinName sclPin;

    Timer timer;
	Si7021 *sensor = NULL;
	bool isEnabled = false;
    int measurementIntervalMs = 5000;
};

#endif
