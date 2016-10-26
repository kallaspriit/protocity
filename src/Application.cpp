#include "mbed.h"
#include "rtos.h"

#include "Application.hpp"
#include "Config.hpp"
#include "Debug.hpp"
#include "CommandManager.hpp"
#include "EthernetManager.hpp"

Application::Application(Config *config) :
	config(config)
{}

void Application::run() {
	setup();

	while (true) {
		loop();
	}
}

void Application::setup() {
	setupSerial();
	setupDebug();
	setupCommandManager();
	setupEthernetManager();
}

void Application::loop() {
	printf("> main loop\n");

	Thread::wait(1000);
}

void Application::setupDebug() {
	debug = new Debug();

	debug->setLedMode(LED_BREATHE_INDEX, Debug::LedMode::BREATHE);
}

void Application::setupCommandManager() {
	commandManager = new CommandManager();
}

void Application::setupEthernetManager() {
	ethernetManager = new EthernetManager();

	debug->setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_FAST);

	bool isConnected = ethernetManager->initialize();

	if (isConnected) {
		debug->setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::ON);
	} else {
		debug->setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::OFF);
	}
}

void Application::setupSerial() {
	serial = new Serial(config->serialTxPin, config->serialRxPin);
	serial->baud(config->serialBaudRate);

	serial->attach(this, &Application::handleSerialRx, Serial::RxIrq);

	printf("-- initializing --\n");
}

void Application::handleSerialRx() {
	char receivedChar = serial->getc();

	if (receivedChar == '\n') {
		if (commandManager != NULL) {
			commandManager->handleCommand(commandBuffer);
		} else {
			printf("> command manager is not yet available to handle '%s'", commandBuffer.c_str());
		}

		commandBuffer = "";

		debug->setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);
	} else {
		commandBuffer += receivedChar;
	}
}
