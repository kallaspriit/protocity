#ifndef ABSTRACTCAPABILITY_HPP
#define ABSTRACTCAPABILITY_HPP

#include "CommandManager.hpp"

#include <string>

class PortController;

class AbstractCapability {

public:
	AbstractCapability(PortController *portController);

	virtual std::string getName() = 0;
	virtual CommandManager::Command::Response execute(CommandManager::Command *command) = 0;
	virtual void update(int deltaUs) {};

protected:
	PortController *portController;

};

#endif
