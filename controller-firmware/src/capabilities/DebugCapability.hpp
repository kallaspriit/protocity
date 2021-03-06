#ifndef DEBUGCAPABILITY_HPP
#define DEBUGCAPABILITY_HPP

#include "../AbstractCapability.hpp"
#include "Log.hpp"

class DebugCapability : public AbstractCapability {

public:
	DebugCapability(Serial *serial, PortController *portController, PinName sdaPin = p9, PinName sclPin = p10);

	std::string getName();

	CommandManager::Command::Response handleCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleI2CCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleLoggingCommand(CommandManager::Command *command);

private:
	Log log = Log::getLog("DebugCapability");

    PinName sdaPin;
    PinName sclPin;

};

#endif
