#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "mbed.h"
#include "rtos.h"

#include "SocketServer.hpp"

#include <string>

class Config;
class Debug;
class CommandManager;
class EthernetManager;

class Application : SocketServer::MessageListener {

public:
	Application(Config *config);

	void run();

private:
	void setup();
	void loop();

	void setupSerial();
	void setupDebug();
	void setupCommandManager();
	void setupEthernetManager();
	void setupSocketServer();

	void handleSerialRx();

	void onSocketMessageReceived(std::string message);

	const int LED_BREATHE_INDEX = 0;
	const int LED_COMMAND_RECEIVED_INDEX = 1;
	const int LED_ETHERNET_STATUS_INDEX = 2;

	Config *config = NULL;
	Serial *serial = NULL;
	Debug *debug = NULL;
	CommandManager *commandManager = NULL;
	EthernetManager *ethernetManager = NULL;
	SocketServer *socketServer = NULL;

	std::string commandBuffer;
};

#endif
