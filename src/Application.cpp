#include "Application.hpp"
#include "Config.hpp"

Application::Application(Config *config) :
	config(config),
	serial(config->serialTxPin, config->serialRxPin)
{}

void Application::run() {
	setup();

	while (true) {
		loop();
	}
}

void Application::setup() {
	setupTimer();
	setupSerial();
	setupCommandHandlers();
	setupDebug();
	setupEthernetManager();
	setupSocketServer();
}

void Application::loop() {
	// nothing for now

	Thread::wait(1000);
}

void Application::setupTimer() {
	timer.start();
}

void Application::setupSerial() {
	serial.baud(config->serialBaudRate);
	serial.attach(this, &Application::handleSerialRx, Serial::RxIrq);

	printf("\n\n-- initializing --\n");
}

void Application::setupCommandHandlers() {
	registerCommandHandler("memory", this, &Application::handleMemoryCommand);
	registerCommandHandler("sum", this, &Application::handleSumCommand);
	registerCommandHandler("led", this, &Application::handleLedCommand);
}

void Application::setupDebug() {
	debug.setLedMode(LED_BREATHE_INDEX, Debug::LedMode::BREATHE);
}

void Application::setupEthernetManager() {
	debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_FAST);

	bool isConnected = ethernetManager.initialize();

	if (isConnected) {
		debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_SLOW);
	} else {
		debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::OFF);
	}
}

void Application::setupSocketServer() {
	socketServer.addListener(this);
	socketServer.start(ethernetManager.getEthernetInterface(), config->socketServerPort);
}

template<typename T, typename M>
void Application::registerCommandHandler(std::string name, T *obj, M method) {
	registerCommandHandler(name, Callback<void(CommandManager::Command*)>(obj, method));
}

void Application::registerCommandHandler(std::string name, Callback<void(CommandManager::Command*)> func) {
	commandHandlerMap[name].attach(func);

	printf("> registering command handler for '%s'\n", name.c_str());
}

void Application::consumeQueuedCommands() {
	CommandManager::Command *command = commandManager.getNextCommand();

	while (command != NULL) {
		consumeCommand(command);

		command = commandManager.getNextCommand();
	}
}

void Application::consumeCommand(CommandManager::Command *command) {
	CommandHandlerMap::iterator commandIt = commandHandlerMap.find(command->name);

	if (commandIt != commandHandlerMap.end()) {
		printf("> calling command handler for '%s'\n", command->name.c_str());

		for (int i = 0; i < command->argumentCount; i++) {
			printf("    argument %d: %s\n", i, command->arguments[i].c_str());
		}

		commandIt->second.call(command);
	} else {
		printf("> command handler for '%s' has not been registered\n", command->name.c_str());

		for (int i = 0; i < command->argumentCount; i++) {
			printf("  argument %d: %s\n", i, command->arguments[i].c_str());
		}
	}
}

void Application::validateCommandArgumentCount(CommandManager::Command *command, int expectedArgumentCount) {
	if (command->argumentCount != expectedArgumentCount) {
		error("> command '%s' expects %d arguments but %d was provided\n", command->name.c_str(), expectedArgumentCount, command->argumentCount);
	}
}

void Application::onSocketClientConnected(TCPSocketConnection* client) {
	debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::ON);
}

void Application::onSocketClientDisconnected(TCPSocketConnection* client) {
	debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_SLOW);
}

void Application::onSocketCommandReceived(const char *command, int length) {
	commandManager.handleCommand(command, length);

	consumeQueuedCommands();

	debug.setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);
}

void Application::handleSerialRx() {
	char receivedChar = serial.getc();

	if (receivedChar == '\n') {
		commandManager.handleCommand(commandBuffer, commandLength);

		commandBuffer[0] = '\0';
		commandLength = 0;

		debug.setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);

		consumeQueuedCommands();
	} else {
		if (commandLength > MAX_COMMAND_LENGTH - 1) {
			error("maximum command length is %d characters, stopping at %s\n", MAX_COMMAND_LENGTH, commandBuffer);
		}

		commandBuffer[commandLength++] = receivedChar;
		commandBuffer[commandLength] = '\0';
	}
}

void Application::handleMemoryCommand(CommandManager::Command *command) {
	validateCommandArgumentCount(command, 0);

	int freeMemoryBytes = Debug::getFreeMemoryBytes();

	printf("> free memory: %d bytes\n", freeMemoryBytes);
}

void Application::handleSumCommand(CommandManager::Command *command) {
	validateCommandArgumentCount(command, 2);

	int a = command->getInt(0);
	int b = command->getInt(1);
	int sum = a + b;

	printf("> sum of %d+%d=%d\n", a, b, sum);
}

void Application::handleLedCommand(CommandManager::Command *command) {
	validateCommandArgumentCount(command, 2);

	int ledIndex = command->getInt(0);
	std::string ledModeRequest = command->getString(1);

	if (ledIndex < 0 || ledIndex > 3) {
		error("> expected led index between 0 and 3");
	}

	Debug::LedMode ledMode = Debug::LedMode::OFF;

	if (ledModeRequest == "OFF") {
		ledMode = Debug::LedMode::OFF;
	} else if (ledModeRequest == "ON") {
		ledMode = Debug::LedMode::ON;
	} else if (ledModeRequest == "BLINK_SLOW") {
		ledMode = Debug::LedMode::BLINK_SLOW;
	} else if (ledModeRequest == "BLINK_FAST") {
		ledMode = Debug::LedMode::BLINK_FAST;
	} else if (ledModeRequest == "BLINK_ONCE") {
		ledMode = Debug::LedMode::BLINK_ONCE;
	} else if (ledModeRequest == "BREATHE") {
		ledMode = Debug::LedMode::BREATHE;
	} else {
		error("unsupported led mode '%s' requested for led %d", ledModeRequest.c_str(), ledIndex);
	}

	debug.setLedMode(ledIndex, ledMode);

	printf("> setting led %d to %s\n", ledIndex, ledModeRequest.c_str());
}
