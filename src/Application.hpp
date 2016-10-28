#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "mbed.h"
#include "rtos.h"
#include "Callback.h"

#include "Debug.hpp"
#include "CommandManager.hpp"
#include "EthernetManager.hpp"
#include "SocketServer.hpp"

#include <map>

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
	void setupCommandHandlers();
	void setupDebug();
	void setupCommandManager();
	void setupEthernetManager();
	void setupSocketServer();

	void sendMessage(const char *fmt, ...);

	template<typename T, typename M>
	void registerCommandHandler(std::string name, T *obj, M method);
	void registerCommandHandler(std::string name, Callback<void(CommandManager::Command*)> func);
	void consumeQueuedCommands();
	void consumeCommand(CommandManager::Command *command);
	bool validateCommandArgumentCount(CommandManager::Command *command, int expectedArgumentCount);

	void handleSerialRx();

	void handleMemoryCommand(CommandManager::Command *command);
	void handleSumCommand(CommandManager::Command *command);
	void handleLedCommand(CommandManager::Command *command);

	void onSocketClientConnected(TCPSocketConnection* client);
	void onSocketClientDisconnected(TCPSocketConnection* client);
	void onSocketCommandReceived(const char *command, int length);

	const int LED_BREATHE_INDEX = 0;
	const int LED_COMMAND_RECEIVED_INDEX = 1;
	const int LED_ETHERNET_STATUS_INDEX = 2;
	static const int MAX_COMMAND_LENGTH = 64;
	static const int SEND_BUFFER_SIZE = 64;

	Config *config = NULL;
	Serial serial;
	Debug debug;
	CommandManager commandManager;
	EthernetManager ethernetManager;
	SocketServer socketServer;
	Timer timer;

	char commandBuffer[MAX_COMMAND_LENGTH + 1];
	char sendBuffer[SEND_BUFFER_SIZE];
	int commandLength = 0;

	typedef std::map<std::string, Callback<void(CommandManager::Command*)>> CommandHandlerMap;
	CommandHandlerMap commandHandlerMap;
};

#endif
