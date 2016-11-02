#include "Application.hpp"
#include "Config.hpp"

Application::Application(Config *config) :
	config(config),
	serial(config->serialTxPin, config->serialRxPin),
	digitalPort1(1, config->digitalPort1Pin)
{}

void Application::run() {
	setup();
	testSetup();

	while (true) {
		loop();
	}
}

void Application::setup() {
	setupSerial();
	setupCommandHandlers();
	setupPorts();
	setupDebug();
	setupEthernetManager();
	setupSocketServer();
}

void Application::loop() {
	consumeQueuedCommands();

	// Thread::wait(1000);
}

void Application::setupSerial() {
	// configure serial
	serial.baud(config->serialBaudRate);
	serial.attach(this, &Application::handleSerialRx, Serial::RxIrq);

	printf("\n\n### initializing ###\n");
}

void Application::setupCommandHandlers() {
	printf("# setting up command handlers\n");

	// register command handlers
	registerCommandHandler("memory", this, &Application::handleMemoryCommand);
	registerCommandHandler("sum", this, &Application::handleSumCommand);
	registerCommandHandler("led", this, &Application::handleLedCommand);
	registerCommandHandler("digitalport", this, &Application::handleDigitalPortCommand);
}

void Application::setupPorts() {
	printf("# setting up ports\n");

	// register controller in the list
	controllerList.push_back(&digitalPort1); // TODO is this used?

	// register the ports in the mapping
	digitalPortNumberToControllerMap[digitalPort1.getId()] = &digitalPort1;
}

void Application::setupDebug() {
	printf("# setting up debugging\n");

	debug.setLedMode(LED_BREATHE_INDEX, Debug::LedMode::BREATHE);
}

void Application::setupEthernetManager() {
	printf("# setting up ethernet manager\n");

	debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_FAST);

	bool isConnected = ethernetManager.initialize();

	if (isConnected) {
		debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_SLOW);
	} else {
		debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::OFF);
	}
}

void Application::setupSocketServer() {
	printf("# setting up socket server\n");

	socketServer.addListener(this);
	socketServer.start(ethernetManager.getEthernetInterface(), config->socketServerPort);
}

template<typename T, typename M>
void Application::registerCommandHandler(std::string name, T *obj, M method) {
	registerCommandHandler(name, Callback<CommandManager::Command::Response(CommandManager::Command*)>(obj, method));
}

void Application::registerCommandHandler(std::string name, Callback<CommandManager::Command::Response(CommandManager::Command*)> func) {
	printf("# registering command handler for '%s'\n", name.c_str());

	commandHandlerMap[name].attach(func);
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

	std::string responseText;

	if (commandIt != commandHandlerMap.end()) {
		printf("# calling command handler for #%d '%s' (source: %d)\n", command->id, command->name.c_str(), command->sourceId);

		for (int i = 0; i < command->argumentCount; i++) {
			printf("#  argument %d: %s\n", i, command->arguments[i].c_str());
		}

		CommandManager::Command::Response response = commandIt->second.call(command);
		responseText = response.getResponseText();

		printf("# command response for %d: %s\n", response.requestId, responseText.c_str());
	} else {
		printf("# command handler for #%d '%s' (source: %d) has not been registered\n", command->id, command->name.c_str(), command->sourceId);

		for (int i = 0; i < command->argumentCount; i++) {
			printf("#  argument %d: %s\n", i, command->arguments[i].c_str());
		}

		// build response text
		char responseBuffer[100];
		snprintf(responseBuffer, sizeof(responseBuffer), "%d:ERROR:unsupported command requested", command->id);
		responseText = responseBuffer;
	}

	switch (command->sourceId) {
		case CommandSource::SOCKET:
			socketServer.sendMessage(responseText + "\n");
			printf("> %s\n", responseText.c_str());
			break;

		case CommandSource::SERIAL:
			printf("> %s\n", responseText.c_str());
			break;

		default:
			error("unexpected command source %d\n", command->sourceId);
	}
}

bool Application::validateCommandArgumentCount(CommandManager::Command *command, int expectedArgumentCount) {
	if (command->argumentCount != expectedArgumentCount) {
		return false;
	}

	return true;
}

void Application::onSocketClientConnected(TCPSocketConnection* client) {
	debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::ON);
}

void Application::onSocketClientDisconnected(TCPSocketConnection* client) {
	debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_SLOW);
}

void Application::onSocketCommandReceived(const char *command, int length) {
	commandManager.handleCommand(CommandSource::SOCKET, command, length);

	//consumeQueuedCommands();

	debug.setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);
}

void Application::handleSerialRx() {
	char receivedChar = serial.getc();

	if (receivedChar == '\n') {
		commandManager.handleCommand(CommandSource::SERIAL, commandBuffer, commandLength);

		commandBuffer[0] = '\0';
		commandLength = 0;

		debug.setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);

		//consumeQueuedCommands();
	} else {
		if (commandLength > MAX_COMMAND_LENGTH - 1) {
			error("maximum command length is %d characters, stopping at %s\n", MAX_COMMAND_LENGTH, commandBuffer);
		}

		commandBuffer[commandLength++] = receivedChar;
		commandBuffer[commandLength] = '\0';
	}
}

CommandManager::Command::Response Application::handleMemoryCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 0)) {
		return command->createFailureResponse("expecing no parameters");
	}

	int freeMemoryBytes = Debug::getFreeMemoryBytes();

	return command->createSuccessResponse(freeMemoryBytes);
}

CommandManager::Command::Response Application::handleSumCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 2)) {
		return command->createFailureResponse("expected two parameters");
	}

	float a = command->getFloat(0);
	float b = command->getFloat(1);
	float sum = a + b;

	return command->createSuccessResponse(sum);
}

CommandManager::Command::Response Application::handleLedCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 2)) {
		return command->createFailureResponse("expected two parameters");
	}

	int ledIndex = command->getInt(0);
	std::string ledModeRequest = command->getString(1);

	if (ledIndex < 0 || ledIndex > 3) {
		return command->createFailureResponse("expected led index between 0 and 3");
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
		return command->createFailureResponse("unsupported led mode requested");
	}

	debug.setLedMode(ledIndex, ledMode);

	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handleDigitalPortCommand(CommandManager::Command *command) {
	if (command->argumentCount < 2) {
		return command->createFailureResponse("expected at least two parameters");
	}

	std::string action = command->getString(1);

	if (action == "mode") {
		return handleDigitalPortModeCommand(command);
	} else {
		return command->createFailureResponse("invalid action requested");
	}

	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handleDigitalPortModeCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 3)) {
		return command->createFailureResponse("expected three parameters");
	}

	int portNumber = command->getInt(0);
	std::string mode = command->getString(2);

	DigitalPortNumberToControllerMap::iterator findIterator = digitalPortNumberToControllerMap.find(portNumber);

	if (findIterator == digitalPortNumberToControllerMap.end()) {
		return command->createFailureResponse("invalid port number requested");
	}

	DigitalPortController *digitalPortController = findIterator->second;

	DigitalPortController::PortMode portMode;

	if (mode == "OUTPUT") {
		portMode = DigitalPortController::PortMode::OUTPUT;
	} else if (mode == "INPUT") {
		portMode = DigitalPortController::PortMode::INPUT;
	} else if (mode == "PWM") {
		portMode = DigitalPortController::PortMode::PWM;
	} else {
		return command->createFailureResponse("invalid port mode requested");
	}

	digitalPortController->setMode(portMode);

	printf("# digitalport set mode for %d: %s\n", portNumber, mode.c_str());

	return command->createSuccessResponse();
}

void Application::testSetup() {
	printf("# setting up tests\n");

	testLoopThread.start(this, &Application::testLoop);
}

void Application::testLoop() {
	while (true) {
		// printf("# test loop %d!\n", testFlipFlop);

		// test digital port
		digitalPort1.setValue(testFlipFlop == 1 ? DigitalPortController::DigitalValue::HIGH : DigitalPortController::DigitalValue::LOW);

		// update loop
		testFlipFlop = testFlipFlop == 1 ? 0 : 1;
		Thread::wait(1000);
	}
}
