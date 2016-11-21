#include "Application.hpp"
#include "Config.hpp"

Application::Application(Config *config) :
	config(config),
	serial(config->serialTxPin, config->serialRxPin),
	port1(1, config->port1Pin),
	port2(2, config->port2Pin),
	port3(3, config->port3Pin),
	port4(4, config->port4Pin),
	port5(5, config->port5Pin),
	port6(6, config->port6Pin)
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
	setupTimer();
}

void Application::loop() {
	int deltaUs = timer.read_us();
	timer.reset();

	consumeQueuedCommands();
	sendQueuedMessages();
	updateControllers(deltaUs);
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
	registerCommandHandler("port", this, &Application::handlePortCommand);
}

void Application::setupPorts() {
	printf("# setting up ports\n");

	// register the ports in the mapping
	portNumberToControllerMap[port1.getId()] = &port1;
	portNumberToControllerMap[port2.getId()] = &port2;
	portNumberToControllerMap[port3.getId()] = &port3;
	portNumberToControllerMap[port4.getId()] = &port4;
	portNumberToControllerMap[port5.getId()] = &port5;
	portNumberToControllerMap[port6.getId()] = &port6;

	// register port event listeners
	for (DigitalPortNumberToControllerMap::iterator it = portNumberToControllerMap.begin(); it != portNumberToControllerMap.end(); it++) {
		it->second->addEventListener(this);
	}
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

void Application::setupTimer() {
	timer.start();
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

void Application::sendQueuedMessages() {
	while (messageQueue.size() > 0) {
		std::string message = messageQueue.front();
		messageQueue.pop();

		printf(message.c_str());
		socketServer.sendMessage(message);
	}
}

void Application::updateControllers(int deltaUs) {
	for (DigitalPortNumberToControllerMap::iterator it = portNumberToControllerMap.begin(); it != portNumberToControllerMap.end(); it++) {
		it->second->update(deltaUs);
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

void Application::onPortDigitalValueChange(int id, PortController::DigitalValue value) {
	snprintf(sendBuffer, SEND_BUFFER_SIZE, "0:INTERRUPT:%d:%s\n", id, value == PortController::DigitalValue::HIGH ? "HIGH" : "LOW");

	messageQueue.push(std::string(sendBuffer));

	// TODO remove temporary test
	port1.setPortMode(PortController::PortMode::OUTPUT);
	port1.setValue(value);
}

void Application::onPortAnalogValueChange(int id, float value) {
	snprintf(sendBuffer, SEND_BUFFER_SIZE, "0:ANALOG:%d:%f\n", id, value);

	messageQueue.push(std::string(sendBuffer));

	// TODO remove temporary test
	port2.setPortMode(PortController::PortMode::PWM);
	port2.setPwmDutyCycle(value);
}

void Application::onPortValueRise(int id) {

}

void Application::onPortValueFall(int id) {

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

CommandManager::Command::Response Application::handlePortCommand(CommandManager::Command *command) {
	if (command->argumentCount < 2) {
		return command->createFailureResponse("expected at least two parameters");
	}

	std::string action = command->getString(1);

	if (action == "mode") {
		return handlePortModeCommand(command);
	} else if (action == "pull") {
		return handlePortPullCommand(command);
	} else if (action == "value") {
		return handlePortValueCommand(command);
	} else if (action == "read") {
		return handlePortReadCommand(command);
	} else if (action == "listen") {
		return handlePortListenCommand(command);
	} else {
		return command->createFailureResponse("invalid action requested");
	}

	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handlePortModeCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 3)) {
		return command->createFailureResponse("expected three parameters");
	}

	int portNumber = command->getInt(0);

	PortController *portController = getPortControllerByPortNumber(portNumber);

	if (portController == NULL) {
		return command->createFailureResponse("invalid port number requested");
	}

	std::string modeName = command->getString(2);
	PortController::PortMode portMode = PortController::getPortModeByName(modeName);

	if (portMode == PortController::PortMode::INVALID) {
		return command->createFailureResponse("invalid port mode requested");
	}

	portController->setPortMode(portMode);

	printf("# set port %d mode: %s\n", portNumber, modeName.c_str());

	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handlePortPullCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 3)) {
		return command->createFailureResponse("expected three parameters");
	}

	int portNumber = command->getInt(0);
	PortController *portController = getPortControllerByPortNumber(portNumber);

	if (portController == NULL) {
		return command->createFailureResponse("invalid port number requested");
	}

	PortController::PortMode portMode = portController->getPortMode();

	if (portMode != PortController::PortMode::INPUT && portMode != PortController::PortMode::INTERRUPT) {
		return command->createFailureResponse("setting pull mode is only applicable for INPUT and INTERRUPT ports");
	}

	std::string modeName = command->getString(2);
	PinMode pinMode = PinMode::PullNone;

	if (modeName == "NONE") {
		pinMode = PinMode::PullNone;
	} else if (modeName == "UP") {
		pinMode = PinMode::PullUp;
	} else if (modeName == "DOWN") {
		pinMode = PinMode::PullDown;
	} else {
		return command->createFailureResponse("invalid pull mode requested");
	}

	portController->setPinMode(pinMode);

	printf("# set port %d pull mode: %s\n", portNumber, modeName.c_str());

	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handlePortValueCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 3)) {
		return command->createFailureResponse("expected three parameters");
	}

	int portNumber = command->getInt(0);
	float value = command->getFloat(2);

	PortController *portController = getPortControllerByPortNumber(portNumber);

	if (portController == NULL) {
		return command->createFailureResponse("invalid port number requested");
	}

	PortController::PortMode portMode = portController->getPortMode();

	switch (portMode) {
		case PortController::PortMode::OUTPUT: {
			PortController::DigitalValue digitalValue = PortController::DigitalValue::LOW;
			std::string stringValue = command->getString(2);

			if (stringValue == "HIGH") {
				digitalValue = PortController::DigitalValue::HIGH;
			} else if (stringValue == "LOW") {
				digitalValue = PortController::DigitalValue::LOW;
			} else {
				if (value != 0.0f && value != 1.0f) {
					return command->createFailureResponse("expected either HIGH/LOW or 1/0 as value");
				}

				digitalValue = value == 1.0f
					? PortController::DigitalValue::HIGH
					: PortController::DigitalValue::LOW;
			}

			portController->setValue(digitalValue);

			printf("# port set digital value for %d: %d\n", portNumber, digitalValue);
		}
		break;

		case PortController::PortMode::PWM: {
			if (value < 0.0f || value > 1.0f) {
				return command->createFailureResponse("expected value between 0.0 and 1.0");
			}

			float pwmDutyCycle = min(max(value, 0.0f), 1.0f);

			portController->setPwmDutyCycle(pwmDutyCycle);

			printf("# port set pwm duty cycle value for %d: %f\n", portNumber, pwmDutyCycle);
		}
		break;

		default:
			return command->createFailureResponse("setting port value is only valid for OUTPUT or PWM modes");
	}


	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handlePortReadCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 2)) {
		return command->createFailureResponse("expected two parameters");
	}

	int portNumber = command->getInt(0);

	PortController *portController = getPortControllerByPortNumber(portNumber);

	if (portController == NULL) {
		return command->createFailureResponse("invalid port number requested");
	}

	PortController::PortMode portMode = portController->getPortMode();

	if (portMode == PortController::PortMode::INPUT) {
		PortController::DigitalValue value = portController->getDigitalValue();

		return command->createSuccessResponse(value == PortController::DigitalValue::HIGH ? "HIGH" : "LOW");
	} else if (portMode == PortController::PortMode::ANALOG) {
		float value = portController->getAnalogValue();

		return command->createSuccessResponse(value);
	} else {
		return command->createFailureResponse("reading value is only valid for digital or analog inputs");
	}
}

CommandManager::Command::Response Application::handlePortListenCommand(CommandManager::Command *command) {
	if (command->argumentCount < 2 || command->argumentCount > 4) {
		return command->createFailureResponse("expected at least two and no more than four parameters");
	}

	int portNumber = command->getInt(0);

	PortController *portController = getPortControllerByPortNumber(portNumber);

	if (portController == NULL) {
		return command->createFailureResponse("invalid port number requested");
	}

	PortController::PortMode portMode = portController->getPortMode();

	if (portMode != PortController::PortMode::ANALOG) {
		return command->createFailureResponse("listening for port events is only valid for analog inputs");
	}

	float changeThreshold = 0.01f;
	int intervalMs = 0;

	if (command->argumentCount >= 3) {
		if (command->getString(2) == "off") {
			printf("# stopping listening for analog port %d value changes\n", portNumber);

			portController->stopAnalogValueListener();

			return command->createSuccessResponse();
		}

		changeThreshold = command->getFloat(2);
	}

	if (command->argumentCount >= 4) {
		intervalMs = command->getInt(3);
	}

	printf("# listening for analog port %d value changes (threshold: %f, interval: %dms)\n", portNumber, changeThreshold, intervalMs);

	portController->listenAnalogValueChange(changeThreshold, intervalMs);

	return command->createSuccessResponse();
}

PortController *Application::getPortControllerByPortNumber(int portNumber) {
	DigitalPortNumberToControllerMap::iterator findIterator = portNumberToControllerMap.find(portNumber);

	if (findIterator == portNumberToControllerMap.end()) {
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
		// port1.setValue(testFlipFlop == 1 ? PortController::DigitalValue::HIGH : PortController::DigitalValue::LOW);

		// update loop
		testFlipFlop = testFlipFlop == 1 ? 0 : 1;
		Thread::wait(1000);
	}
}
