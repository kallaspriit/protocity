#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "mbed.h"
#include "rtos.h"

#include <string>

class Config;
class Debug;
class CommandManager;

class Application {

public:
	Application(Config *config);

	void run();

private:
	void setup();
	void loop();

	void setupDebug();
	void setupCommandManager();
	void setupSerial();

	void handleSerialRx();
	void handleSerialTx();

	Config *config = NULL;
	Debug *debug = NULL;
	CommandManager *commandManager = NULL;
	Serial *serial = NULL;

	std::string commandBuffer;
};

#endif
