#include "Application.hpp"
#include "Config.hpp"

#include "capabilities/DebugCapability.hpp"
#include "capabilities/TSL2561Capability.hpp"
#include "capabilities/TMP102Capability.hpp"
#include "capabilities/MPL3115A2Capability.hpp"
#include "capabilities/PN532Capability.hpp"
#include "capabilities/TLC5940Capability.hpp"
#include "capabilities/Si7021Capability.hpp"
#include "capabilities/WeatherStationCapability.hpp"

Application::Application(Config *config, Serial *serial) :
	config(config),
	serial(serial),
	port1(1, config->port1Pin),
	port2(2, config->port2Pin),
	port3(3, config->port3Pin),
	port4(4, config->port4Pin),
	port5(5, config->port5Pin),
	port6(6, config->port6Pin),
	port7(7, config->port7Pin),
	mainLoopLed(config->mainLoopLedPin)
{
	commandBuffer = new char[COMMAND_BUFFER_SIZE];
	sendBuffer = new char[SEND_BUFFER_SIZE];
}

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

	initialFreeMemory = Debug::getFreeMemoryBytes();
}

void Application::loop() {
	int deltaUs = timer.read_us();
	timer.reset();

	consumeQueuedCommands();
	sendQueuedMessages();
	updateControllers(deltaUs);
	updateHeartbeat(deltaUs);

	mainLoopLed = !mainLoopLed;
}

void Application::setupSerial() {
	// configure serial
	serial->attach(callback(this, &Application::handleSerialRx), Serial::RxIrq);
}

void Application::setupCommandHandlers() {
	log.info("setting up command handlers");

	// register command handlers
	registerCommandHandler("ping", this, &Application::handlePingCommand);
	registerCommandHandler("memory", this, &Application::handleMemoryCommand);
	registerCommandHandler("version", this, &Application::handleVersionCommand);
	registerCommandHandler("restart", this, &Application::handleRestartCommand);
	registerCommandHandler("port", this, &Application::handlePortCommand);
}

void Application::setupPorts() {
	log.info("setting up ports");

	// register the ports in the mapping
	portNumberToControllerMap[port1.getId()] = &port1;
	portNumberToControllerMap[port2.getId()] = &port2;
	portNumberToControllerMap[port3.getId()] = &port3;
	portNumberToControllerMap[port4.getId()] = &port4;
	portNumberToControllerMap[port5.getId()] = &port5;
	portNumberToControllerMap[port6.getId()] = &port6;
	portNumberToControllerMap[port7.getId()] = &port7;

	// register port event listeners
	for (DigitalPortNumberToControllerMap::iterator it = portNumberToControllerMap.begin(); it != portNumberToControllerMap.end(); it++) {
		setupPort(it->second);
	}
}

void Application::setupPort(PortController *portController) {
	portController->addEventListener(this);

	portController->addCapability(new DebugCapability(serial, portController, config->sdaPin, config->sclPin));
	portController->addCapability(new TSL2561Capability(serial, portController, config->sdaPin, config->sclPin));
	portController->addCapability(new TMP102Capability(serial, portController, config->sdaPin, config->sclPin));
	portController->addCapability(new MPL3115A2Capability(serial, portController, config->sdaPin, config->sclPin));
	portController->addCapability(new Si7021Capability(serial, portController, config->sdaPin, config->sclPin));
	portController->addCapability(new PN532Capability(serial, portController, config->nfcMosiPin, config->nfcMisoPin, config->nfcSclkPin));
	portController->addCapability(new TLC5940Capability(serial, portController, config->ledMosiPin, config->ledSclkPin, config->ledBlankPin, config->ledErrorPin, config->ledGsclkPin, config->ledChainLength));
	portController->addCapability(new WeatherStationCapability(serial, portController, config->sdaPin, config->sclPin, config->lcdTxPin, config->lcdRxPin, config->lcdResetPin));
}

void Application::setupDebug() {
	log.info("setting up debugging");

	debug.setLedMode(LED_BREATHE_INDEX, Debug::LedMode::BREATHE);
}

void Application::setupEthernetManager() {
	log.info("setting up ethernet manager");

	debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_FAST);

	bool isConnected = ethernetManager.initialize();

	if (isConnected) {
		debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_SLOW);
	} else {
		debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::OFF);

		log.warn("connecting to ethernet failed, performing reset to try again");

		restart();
	}
}

void Application::setupSocketServer() {
	log.info("setting up socket server");

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
	log.info("registering command handler for '%s'", name.c_str());

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
		log.trace("calling command handler for #%d '%s' (source: %d)", command->id, command->name.c_str(), command->sourceId);

		for (int i = 0; i < command->argumentCount; i++) {
			log.trace("- argument %d: %s", i, command->arguments[i].c_str());
		}
		*/

		CommandManager::Command::Response response = commandIt->second.call(command);
		responseText = response.getResponseText();
	} else {
		log.warn("command handler for #%d '%s' (source: %d) has not been registered", command->id, command->name.c_str(), command->sourceId);

		for (int i = 0; i < command->argumentCount; i++) {
			log.debug("- argument %d: %s", i, command->arguments[i].c_str());
		}

		// build response text
		char responseBuffer[100];
		snprintf(responseBuffer, sizeof(responseBuffer), "%d:ERROR:unsupported command requested", command->id);
		responseText = responseBuffer;
	}

	log.debug("> %s", responseText.c_str());

	switch (command->sourceId) {
		case CommandSource::SOCKET:
			socketServer.sendMessage(responseText + "\n");
			break;

		case CommandSource::SERIAL:
			break;

		default:
			log.error("unexpected command source %d", command->sourceId);
	}
}

void Application::sendQueuedMessages() {
	while (messageQueue.size() > 0) {
		std::string message = messageQueue.front();
		messageQueue.pop();

		// last character is a linefeed \n, remove it
		log.debug("> %s", message.substr(0, message.length() - 1).c_str());

		if (socketServer.isClientConnected()) {
			socketServer.sendMessage(message);
		}
	}
}

void Application::updateControllers(int deltaUs) {
	for (DigitalPortNumberToControllerMap::iterator it = portNumberToControllerMap.begin(); it != portNumberToControllerMap.end(); it++) {
		it->second->update(deltaUs);
	}
}

void Application::updateHeartbeat(int deltaUs) {
	timeSinceLastHeartbeatUs += deltaUs;

	if (timeSinceLastHeartbeatUs > HEATBEAT_INTERVAL_US) {
		sendHeartbeat();

		timeSinceLastHeartbeatUs = 0;
	}
}

void Application::sendHeartbeat() {
	if (!socketServer.isClientConnected()) {
		return;
	}

	int length = snprintf(sendBuffer, SEND_BUFFER_SIZE, "0:HEARTBEAT:%d\n", heartbeatCounter++);

	socketServer.sendMessage(sendBuffer, length);
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

	log.warn("socket client disconnected, performing reset");

	restart();
}

void Application::onSocketCommandReceived(const char *command, int length) {
	commandManager.handleCommand(CommandSource::SOCKET, command, length);

	debug.setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);
}

void Application::onPortDigitalValueChange(int id, PortController::DigitalValue value) {
	snprintf(sendBuffer, SEND_BUFFER_SIZE, "0:INTERRUPT_CHANGE:%d:%s\n", id, value == PortController::DigitalValue::HIGH ? "HIGH" : "LOW");

	enqueueMessage(std::string(sendBuffer));
}

void Application::onPortAnalogValueChange(int id, float value) {
	snprintf(sendBuffer, SEND_BUFFER_SIZE, "0:ANALOG_IN:%d:%f\n", id, value);

	enqueueMessage(std::string(sendBuffer));
}

void Application::onPortValueRise(int id) {
	snprintf(sendBuffer, SEND_BUFFER_SIZE, "0:INTERRUPT_RISE:%d\n", id);

	enqueueMessage(std::string(sendBuffer));
}

void Application::onPortValueFall(int id) {
	snprintf(sendBuffer, SEND_BUFFER_SIZE, "0:INTERRUPT_FALL:%d\n", id);

	enqueueMessage(std::string(sendBuffer));
}

void Application::onPortCapabilityUpdate(int id, std::string capabilityName, std::string message) {
	snprintf(sendBuffer, SEND_BUFFER_SIZE, "0:CAPABILITY:%d:%s:%s\n", id, capabilityName.c_str(), message.c_str());

	enqueueMessage(std::string(sendBuffer));
}

void Application::handleSerialRx() {
	char receivedChar = serial->getc();

	if (receivedChar == '\n') {
		commandManager.handleCommand(CommandSource::SERIAL, commandBuffer, commandLength);

		commandBuffer[0] = '\0';
		commandLength = 0;

		debug.setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);

		//consumeQueuedCommands();
	} else {
		if (commandLength > MAX_COMMAND_LENGTH - 1) {
			log.warn("maximum command length is %d characters, stopping at %s", MAX_COMMAND_LENGTH, commandBuffer);
		}

		commandBuffer[commandLength++] = receivedChar;
		commandBuffer[commandLength] = '\0';
	}
}

void Application::enqueueMessage(std::string message) {
	messageQueue.push(message);
}

CommandManager::Command::Response Application::handlePingCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 0)) {
		return command->createFailureResponse("expected no parameters");
	}

	return command->createSuccessResponse("pong");
}

CommandManager::Command::Response Application::handleMemoryCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 0)) {
		return command->createFailureResponse("expected no parameters");
	}

	int freeMemoryBytes = Debug::getFreeMemoryBytes();

	return command->createSuccessResponse(freeMemoryBytes, initialFreeMemory, Debug::getTotalMemoryBytes());
}

CommandManager::Command::Response Application::handleVersionCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 0)) {
		return command->createFailureResponse("expected no parameters");
	}

	return command->createSuccessResponse(getVersion());
}

CommandManager::Command::Response Application::handleRestartCommand(CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 0)) {
		return command->createFailureResponse("expected no parameters");
	}

	restart();

	return command->createSuccessResponse();
}

void Application::restart() {
	log.info("restarting system..");

	Thread::wait(100);

	NVIC_SystemReset();
}

CommandManager::Command::Response Application::handlePortCommand(CommandManager::Command *command) {
	if (command->argumentCount < 2) {
		return command->createFailureResponse("expected at least two parameters");
	}

	int portNumber = command->getInt(0);

	PortController *portController = getPortControllerByPortNumber(portNumber);

	if (portController == NULL) {
		return command->createFailureResponse("invalid port number requested");
	}

	std::string action = command->getString(1);

	if (action == "mode") {
		return handlePortModeCommand(portController, command);
	} else if (action == "pull") {
		return handlePortPullCommand(portController, command);
	} else if (action == "value") {
		return handlePortValueCommand(portController, command);
	} else if (action == "read") {
		return handlePortReadCommand(portController, command);
	} else if (action == "listen") {
		return handlePortListenCommand(portController, command);
	} else {
		AbstractCapability *capability = portController->getCapabilityByName(action);

		if (capability == NULL) {
			return command->createFailureResponse("invalid action requested");
		}

		return capability->handleCommand(command);
	}

	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handlePortModeCommand(PortController *portController, CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 3)) {
		return command->createFailureResponse("expected three parameters");
	}

	int portNumber = command->getInt(0);
	std::string modeName = command->getString(2);
	PortController::PortMode portMode = PortController::getPortModeByName(modeName);

	if (portMode == PortController::PortMode::INVALID) {
		return command->createFailureResponse("invalid port mode requested");
	}

	portController->setPortMode(portMode);

	log.debug("set port %d mode: %s", portNumber, modeName.c_str());

	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handlePortPullCommand(PortController *portController, CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 3)) {
		return command->createFailureResponse("expected three parameters");
	}

	PortController::PortMode portMode = portController->getPortMode();

	if (portMode != PortController::PortMode::DIGITAL_IN && portMode != PortController::PortMode::INTERRUPT) {
		return command->createFailureResponse("setting pull mode is only applicable for DIGITAL_IN and INTERRUPT ports");
	}

	int portNumber = command->getInt(0);
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

	log.debug("set port %d pull mode: %s", portNumber, modeName.c_str());

	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handlePortValueCommand(PortController *portController, CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 3)) {
		return command->createFailureResponse("expected three parameters");
	}

	int portNumber = command->getInt(0);
	float value = command->getFloat(2);

	PortController::PortMode portMode = portController->getPortMode();

	switch (portMode) {
		case PortController::PortMode::DIGITAL_OUT: {
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

			portController->setDigitalValue(digitalValue);

			log.debug("port set digital value for %d: %d", portNumber, digitalValue);
		}
		break;

		case PortController::PortMode::ANALOG_OUT: {
			if (value < 0.0f || value > 1.0f) {
				return command->createFailureResponse("expected value between 0.0 and 1.0");
			}

			float pwmDutyCycle = min(max(value, 0.0f), 1.0f);

			portController->setAnalogValue(pwmDutyCycle);

			log.debug("port set pwm duty cycle value for %d: %f", portNumber, pwmDutyCycle);
		}
		break;

		default:
			return command->createFailureResponse("setting port value is only valid for DIGITAL_OUT or ANALOG_OUT modes");
	}


	return command->createSuccessResponse();
}

CommandManager::Command::Response Application::handlePortReadCommand(PortController *portController, CommandManager::Command *command) {
	if (!validateCommandArgumentCount(command, 2)) {
		return command->createFailureResponse("expected two parameters");
	}

	PortController::PortMode portMode = portController->getPortMode();

	if (portMode == PortController::PortMode::DIGITAL_IN || portMode == PortController::PortMode::INTERRUPT) {
		PortController::DigitalValue value = portController->getDigitalValue();

		return command->createSuccessResponse(value == PortController::DigitalValue::HIGH ? "HIGH" : "LOW");
	} else if (portMode == PortController::PortMode::ANALOG_IN) {
		float value = portController->getAnalogValue();

		return command->createSuccessResponse(value);
	} else {
		return command->createFailureResponse("reading value is only valid for DIGITAL_IN, INTERRUPT and ANALOG_IN");
	}
}

CommandManager::Command::Response Application::handlePortListenCommand(PortController *portController, CommandManager::Command *command) {
	if (command->argumentCount < 2 || command->argumentCount > 4) {
		return command->createFailureResponse("expected at least two and no more than four parameters");
	}

	PortController::PortMode portMode = portController->getPortMode();

	if (portMode != PortController::PortMode::ANALOG_IN) {
		return command->createFailureResponse("listening for port events is only valid for analog inputs");
	}

	int portNumber = command->getInt(0);
	float changeThreshold = 0.01f;
	int intervalMs = 0;

	if (command->argumentCount >= 3) {
		if (command->getString(2) == "off") {
			log.info("stopping listening for analog port %d value changes", portNumber);

			portController->stopAnalogValueListener();

			return command->createSuccessResponse();
		}

		changeThreshold = command->getFloat(2);
	}

	if (command->argumentCount >= 4) {
		intervalMs = command->getInt(3);
	}

	log.info("listening for analog port %d value changes (threshold: %f, interval: %dms)", portNumber, changeThreshold, intervalMs);

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
	log.info("setting up tests");

	testLoopThread.start(callback(this, &Application::testLoop));
}

void Application::testLoop() {
	/*
	TSL2561 lum(p9, p10, TSL2561_ADDR_FLOAT);

	if (lum.begin()) {
        log.debug("TSL2561 Sensor Found");
    } else {
        log.debug("TSL2561 Sensor not Found");
    }

	lum.setGain(TSL2561_GAIN_0X);
	lum.setTiming(TSL2561_INTEGRATIONTIME_402MS);

	while (true) {
		// test TSL2561
		// log.info("illuminance: %f lux", lum.lux());

		uint16_t x,y,z;
		x = lum.getLuminosity(TSL2561_VISIBLE);
        y = lum.getLuminosity(TSL2561_FULLSPECTRUM);
        z = lum.getLuminosity(TSL2561_INFRARED);

		log.info("illuminance: %d, %d, %d lux", x, y, z);

		// update loop
		testFlipFlop = testFlipFlop == 1 ? 0 : 1;
		Thread::wait(1000);
	}
	*/
}
