#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "mbed.h"
#include "rtos.h"
#include "Callback.h"

#include "Debug.hpp"
#include "CommandManager.hpp"
#include "EthernetManager.hpp"
#include "SocketServer.hpp"
#include "AbstractController.hpp"
#include "controllers/DigitalPortController.hpp"

#include <map>

class Config;
class TCPSocketConnection;

class Application : SocketServer::SocketServerListener, DigitalPortController::DigitalPortInterruptListener {

public:
	Application(Config *config);

	void run();

private:
	// list of possible command sources
	enum CommandSource {
		SOCKET,
		SERIAL
	};


	// main lifecycle methods
	void setup();					// sets up the system
	void loop();					// main loop that gets continuously called

	// application setup methods
	void setupSerial();
	void setupCommandHandlers();
	void setupPorts();
	void setupDebug();
	void setupCommandManager();
	void setupEthernetManager();
	void setupSocketServer();

	// command handling
	template<typename T, typename M>
	void registerCommandHandler(std::string name, T *obj, M method);
	void registerCommandHandler(std::string name, Callback<CommandManager::Command::Response(CommandManager::Command*)> func);
	void consumeQueuedCommands();
	void consumeCommand(CommandManager::Command *command);
	bool validateCommandArgumentCount(CommandManager::Command *command, int expectedArgumentCount);
	void handleSerialRx();

	// built in command handlers
	CommandManager::Command::Response handleMemoryCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleSumCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleLedCommand(CommandManager::Command *command);

	// digital port command handlers
	CommandManager::Command::Response handleDigitalPortCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDigitalPortModeCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDigitalPortValueCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDigitalPortReadCommand(CommandManager::Command *command);

	// port helpers
	DigitalPortController *getDigitalPortControllerByPortNumber(int portNumber);

	// socket server listeners
	void onSocketClientConnected(TCPSocketConnection* client);
	void onSocketClientDisconnected(TCPSocketConnection* client);
	void onSocketCommandReceived(const char *command, int length);

	// digital port interrupt listeners
	void onDigitalPortChange(int id, DigitalPortController::DigitalValue value);
	void onDigitalPortRise(int id);
	void onDigitalPortFall(int id);

	// custom type definitions
	typedef std::map<std::string, Callback<CommandManager::Command::Response(CommandManager::Command*)>> CommandHandlerMap;
	typedef std::map<int, DigitalPortController*> DigitalPortNumberToControllerMap;

	// configuration
	const int LED_BREATHE_INDEX = 0;
	const int LED_COMMAND_RECEIVED_INDEX = 1;
	const int LED_ETHERNET_STATUS_INDEX = 2;
	static const int MAX_COMMAND_LENGTH = 64;
	static const int SEND_BUFFER_SIZE = 64;

	// services
	Config *config = NULL;
	Serial serial;
	Debug debug;
	CommandManager commandManager;
	EthernetManager ethernetManager;
	SocketServer socketServer;

	// command handling
	char commandBuffer[MAX_COMMAND_LENGTH + 1];
	char sendBuffer[SEND_BUFFER_SIZE];
	int commandLength = 0;
	CommandHandlerMap commandHandlerMap;

	// digital port controllers
	DigitalPortController digitalPort1;
	DigitalPortController digitalPort2;
	DigitalPortController digitalPort3;
	DigitalPortController digitalPort4;
	DigitalPortController digitalPort5;
	DigitalPortController digitalPort6;

	// controller mapping
	DigitalPortNumberToControllerMap digitalPortNumberToControllerMap;

	// test lifecycle methods
	void testSetup();				// sets up the tests
	void testLoop();				// test loop that gets continuously called

	// test helpers
	Thread testLoopThread;
	int testFlipFlop = 0;
};

#endif
