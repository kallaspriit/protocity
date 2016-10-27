#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "mbed.h"
#include "rtos.h"

#include "Debug.hpp"
#include "CommandManager.hpp"
#include "EthernetManager.hpp"
#include "SocketServer.hpp"

#include <string>

class Config;
class EthernetManager;
class TCPSocketConnection;

class Application : SocketServer::SocketServerListener {

public:
	Application(Config *config);

	void run();

private:
	void setup();
	void loop();

	void setupTimer();
	void setupSerial();
	void setupDebug();
	void setupCommandManager();
	void setupEthernetManager();
	void setupSocketServer();

	void handleSerialRx();

	void onSocketClientConnected(TCPSocketConnection* client);
	void onSocketClientDisconnected(TCPSocketConnection* client);
	void onSocketMessageReceived(std::string message);

	const int LED_BREATHE_INDEX = 0;
	const int LED_COMMAND_RECEIVED_INDEX = 1;
	const int LED_ETHERNET_STATUS_INDEX = 2;

	Config *config = NULL;
	Serial serial;
	Debug debug;
	CommandManager commandManager;
	EthernetManager ethernetManager;
	SocketServer socketServer;

	std::string commandBuffer;
	Timer timer;
};

#endif
