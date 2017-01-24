#ifndef ABSTRACTCAPABILITY_HPP
#define ABSTRACTCAPABILITY_HPP

#include "CommandManager.hpp"

#include "mbed.h"
#include "rtos.h"

#include <string>

class PortController;

class AbstractCapability {

public:
	AbstractCapability(Serial *serial, PortController *portController);

	virtual std::string getName() = 0;
	virtual CommandManager::Command::Response execute(CommandManager::Command *command) = 0;
	virtual void update(int deltaUs) {};

protected:
	//void startStepThread();
	//virtual void runThread();
	//virtual void step(int deltaUs);
	//Thread thread;
	//Timer timer;

	Serial *serial;
	PortController *portController;

	static const int SEND_BUFFER_SIZE = 1024;
	char *sendBuffer;

};

#endif
