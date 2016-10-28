#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "mbed.h"
#include "rtos.h"

#include "Debug.hpp"
#include "CommandManager.hpp"
#include "EthernetManager.hpp"
#include "SocketServer.hpp"

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
	void onSocketCommandReceived(const char *command, int length);

	const int LED_BREATHE_INDEX = 0;
	const int LED_COMMAND_RECEIVED_INDEX = 1;
	const int LED_ETHERNET_STATUS_INDEX = 2;
	static const int MAX_COMMAND_LENGTH = 64;

	Config *config = NULL;
	Serial serial;
	Debug debug;
	CommandManager commandManager;
	EthernetManager ethernetManager;
	SocketServer socketServer;

	char commandBuffer[MAX_COMMAND_LENGTH + 1];
	int commandLength = 0;
	Timer timer;
};

#endif
