#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "mbed.h"
#include "rtos.h"

#include "Config.hpp"
#include "CommandManager.hpp"

class Application {

public:
	Application(Config *config);

	void run();


private:
	void setup();
	void loop();

	void setupCommandManager();
	void setupSerial();

	void handleSerialRx();
	void handleSerialTx();

	Config *config = NULL;
	Serial *serial = NULL;
	CommandManager *commandManager;

	std::string commandBuffer;
};

#endif
