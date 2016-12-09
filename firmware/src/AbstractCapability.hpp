#ifndef ABSTRACTCAPABILITY_HPP
#define ABSTRACTCAPABILITY_HPP

#include "CommandManager.hpp"

#include "mbed.h"
#include <string>

class PortController;

class AbstractCapability {

public:
	AbstractCapability(Serial *serial, PortController *portController);

	virtual std::string getName() = 0;
	virtual CommandManager::Command::Response execute(CommandManager::Command *command) = 0;
	virtual void update(int deltaUs) {};

protected:
	Serial *serial;
	PortController *portController;

};

#endif
