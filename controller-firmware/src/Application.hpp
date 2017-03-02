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
#include "PortController.hpp"

#include <map>
#include <queue>

class Config;
class TCPSocketConnection;

class Application : SocketServer::SocketServerListener, PortController::PortEventListener {

public:
	Application(Config *config, Serial *serial);

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
	void setupTimer();

	// command and message handling
	template<typename T, typename M>
	void registerCommandHandler(std::string name, T *obj, M method);
	void registerCommandHandler(std::string name, Callback<CommandManager::Command::Response(CommandManager::Command*)> func);
	void consumeCommand(CommandManager::Command *command);
	bool validateCommandArgumentCount(CommandManager::Command *command, int expectedArgumentCount);
	void handleSerialRx();
	void enqueueMessage(std::string message);

	// loop updates
	void consumeQueuedCommands();
	void sendQueuedMessages();
	void updateControllers(int deltaUs);
	void updateHeartbeat(int deltaUs);

	// heatbeat
	void sendHeartbeat();
	void restart();

	// built in command handlers
	CommandManager::Command::Response handlePingCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleMemoryCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleVersionCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleRestartCommand(CommandManager::Command *command);
	CommandManager::Command::Response handlePortCommand(CommandManager::Command *command);

	// digital port command handlers
	CommandManager::Command::Response handlePortModeCommand(PortController *portController, CommandManager::Command *command);
	CommandManager::Command::Response handlePortPullCommand(PortController *portController, CommandManager::Command *command);
	CommandManager::Command::Response handlePortValueCommand(PortController *portController, CommandManager::Command *command);
	CommandManager::Command::Response handlePortReadCommand(PortController *portController, CommandManager::Command *command);
	CommandManager::Command::Response handlePortListenCommand(PortController *portController, CommandManager::Command *command);

	// port helpers
	void setupPort(PortController *portController);
	PortController *getPortControllerByPortNumber(int portNumber);

	// socket server listeners
	void onSocketClientConnected(TCPSocketConnection* client);
	void onSocketClientDisconnected(TCPSocketConnection* client);
	void onSocketCommandReceived(const char *command, int length);

	// digital port interrupt listeners
	void onPortDigitalValueChange(int id, PortController::DigitalValue value);
	void onPortAnalogValueChange(int id, float value);
	void onPortValueRise(int id);
	void onPortValueFall(int id);
	void onPortCapabilityUpdate(int id, std::string capabilityName, std::string message);

	// custom type definitions
	typedef std::map<std::string, Callback<CommandManager::Command::Response(CommandManager::Command*)>> CommandHandlerMap;
	typedef std::map<int, PortController*> DigitalPortNumberToControllerMap;
	typedef std::queue<std::string> StringQueue;

	// configuration
	const int LED_BREATHE_INDEX = 0;
	const int LED_COMMAND_RECEIVED_INDEX = 1;
	const int LED_ETHERNET_STATUS_INDEX = 2;
	static const int MAX_COMMAND_LENGTH = 1024;
	static const int SEND_BUFFER_SIZE = 1024;
	static const int COMMAND_BUFFER_SIZE = MAX_COMMAND_LENGTH + 1;
	static const int HEATBEAT_INTERVAL_US = 10000000; // 10 seconds

	// services
	Config *config = NULL;
	Serial *serial = NULL;
	Debug debug;
	CommandManager commandManager;
	EthernetManager ethernetManager;
	SocketServer socketServer;

	// command handling
	char *commandBuffer;
	char *sendBuffer;
	int commandLength = 0;
	CommandHandlerMap commandHandlerMap;

	// port controllers
	PortController port1;
	PortController port2;
	PortController port3;
	PortController port4;
	PortController port5;
	PortController port6;

	// updates
	Timer timer;
	DigitalOut mainLoopLed;

	// controller mapping
	DigitalPortNumberToControllerMap portNumberToControllerMap;

	// queued messages
	StringQueue messageQueue;

	// heartbeat
	int timeSinceLastHeartbeatUs = 0;
	int heartbeatCounter = 0;

	// test lifecycle methods
	void testSetup();				// sets up the tests
	void testLoop();				// test loop that gets continuously called

	// test helpers
	Thread testLoopThread;
	int testFlipFlop = 0;
};

#endif
