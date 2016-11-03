#include "Application.hpp"
#include "Config.hpp"

Application::Application(Config *config) :
	config(config),
	serial(config->serialTxPin, config->serialRxPin),
	digitalPort1(1, config->digitalPort1Pin),
	digitalPort2(2, config->digitalPort2Pin),
	digitalPort3(3, config->digitalPort3Pin),
	digitalPort4(4, config->digitalPort4Pin),
	digitalPort5(5, config->digitalPort5Pin),
	digitalPort6(6, config->digitalPort6Pin)
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

	// register the ports in the mapping
	digitalPortNumberToControllerMap[digitalPort1.getId()] = &digitalPort1;
	digitalPortNumberToControllerMap[digitalPort2.getId()] = &digitalPort2;
	digitalPortNumberToControllerMap[digitalPort3.getId()] = &digitalPort3;
	digitalPortNumberToControllerMap[digitalPort4.getId()] = &digitalPort4;
	digitalPortNumberToControllerMap[digitalPort5.getId()] = &digitalPort5;
	digitalPortNumberToControllerMap[digitalPort6.getId()] = &digitalPort6;

	// register interrupt listeners
	digitalPort1.addInterruptListener(this);
	digitalPort2.addInterruptListener(this);
	digitalPort3.addInterruptListener(this);
	digitalPort4.addInterruptListener(this);
	digitalPort5.addInterruptListener(this);
	digitalPort6.addInterruptListener(this);
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
		/*
		printf("# calling command handler for #%d '%s' (source: %d)\n", command->id, command->name.c_str(), command->sourceId);

		for (int i = 0; i < command->argumentCount; i++) {
			printf("#  argument %d: %s\n", i, command->arguments[i].c_str());
		}
		*/

		CommandManager::Command::Response response = commandIt->second.call(command);
		responseText = response.getResponseText();
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

void Application::onDigitalPortChange(int id, DigitalPortController::DigitalValue value) {
	printf("# digital port change %d: %d\n", id, value);

	// TODO temporary
	digitalPort1.setMode(DigitalPortController::PortMode::OUTPUT);
	digitalPort1.setValue(value);
}

void Application::onDigitalPortRise(int id) {

}

void Application::onDigitalPortFall(int id) {

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
	} else if (action == "value") {
		return handleDigitalPortValueCommand(command);
	} else if (action == "read") {
		return handleDigitalPortReadCommand(command);
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

	DigitalPortController *digitalPortController = getDigitalPortControllerByPortNumber(portNumber);

	if (digitalPortController == NULL) {
		return command->createFailureResponse("invalid port number requested");
	}

	DigitalPortController::PortMode portMode = DigitalPortController::getPortModeByName(mode);

	if (portMode == DigitalPortController::PortMode::INVALID) {
		return command->createFailureResponse("invalid port mode requested");
	}

	digitalPortController->setMode(portMode);

	printf("# digitalport set mode for %d: %s\n", portNumber, mode.c_str());

	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handleDigitalPortValueCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 3)) {
		return command->createFailureResponse("expected three parameters");
	}

	int portNumber = command->getInt(0);
	float value = command->getFloat(2);

	DigitalPortController *digitalPortController = getDigitalPortControllerByPortNumber(portNumber);

	if (digitalPortController == NULL) {
		return command->createFailureResponse("invalid port number requested");
	}

	DigitalPortController::PortMode portMode = digitalPortController->getMode();

	switch (portMode) {
		case DigitalPortController::PortMode::OUTPUT: {
			DigitalPortController::DigitalValue digitalValue = DigitalPortController::DigitalValue::LOW;
			std::string stringValue = command->getString(2);

			if (stringValue == "HIGH") {
				digitalValue = DigitalPortController::DigitalValue::HIGH;
			} else if (stringValue == "LOW") {
				digitalValue = DigitalPortController::DigitalValue::LOW;
			} else {
				if (value != 0.0f && value != 1.0f) {
					return command->createFailureResponse("expected either HIGH/LOW or 1/0 as value");
				}

				digitalValue = value == 1.0f
					? DigitalPortController::DigitalValue::HIGH
					: DigitalPortController::DigitalValue::LOW;
			}

			digitalPortController->setValue(digitalValue);

			printf("# digitalport set digital value for %d: %d\n", portNumber, digitalValue);
		}
		break;

		case DigitalPortController::PortMode::PWM: {
			if (value < 0.0f || value > 1.0f) {
				return command->createFailureResponse("expected value between 0.0 and 1.0");
			}

			float pwmDutyCycle = min(max(value, 0.0f), 1.0f);

			digitalPortController->setPwmDutyCycle(pwmDutyCycle);

			printf("# digitalport set pwm duty cycle value for %d: %f\n", portNumber, pwmDutyCycle);
		}
		break;

		default:
			return command->createFailureResponse("setting port value is only valid for OUTPUT or PWM modes");
	}


	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handleDigitalPortReadCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 2)) {
		return command->createFailureResponse("expected two parameters");
	}

	int portNumber = command->getInt(0);

	DigitalPortController *digitalPortController = getDigitalPortControllerByPortNumber(portNumber);

	if (digitalPortController == NULL) {
		return command->createFailureResponse("invalid port number requested");
	}

	DigitalPortController::DigitalValue value = digitalPortController->getDigitalValue();

	return command->createSuccessResponse(value == DigitalPortController::DigitalValue::HIGH ? "HIGH" : "LOW");
}

DigitalPortController *Application::getDigitalPortControllerByPortNumber(int portNumber) {
	DigitalPortNumberToControllerMap::iterator findIterator = digitalPortNumberToControllerMap.find(portNumber);

	if (findIterator == digitalPortNumberToControllerMap.end()) {
		return NULL;
	}

	return findIterator->second;
}




void Application::testSetup() {
	printf("# setting up tests\n");

	testLoopThread.start(this, &Application::testLoop);
}

void Application::testLoop() {
	while (true) {
		// printf("# test loop %d!\n", testFlipFlop);

		// test digital port
		// digitalPort1.setValue(testFlipFlop == 1 ? DigitalPortController::DigitalValue::HIGH : DigitalPortController::DigitalValue::LOW);

		// update loop
		testFlipFlop = testFlipFlop == 1 ? 0 : 1;
		Thread::wait(1000);
	}
}
