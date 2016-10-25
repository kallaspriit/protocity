#include "mbed.h"
#include "rtos.h"

#include "Application.hpp"

Application::Application(Config *config) :
	config(config)
{

}

void Application::run() {
	setup();

	while (true) {
		loop();
	}
}

void Application::setup() {
	setupCommandManager();
	setupSerial();
}

void Application::loop() {
	printf("> main loop\n");

	Thread::wait(1000);
}

void Application::setupCommandManager() {
	commandManager = new CommandManager();
}

void Application::setupSerial() {
	serial = new Serial(config->serialTxPin, config->serialRxPin);
	serial->baud(config->serialBaudRate);

	serial->attach(this, &Application::handleSerialRx, Serial::RxIrq);
	serial->attach(this, &Application::handleSerialTx, Serial::TxIrq);
}

void Application::handleSerialRx() {
	// serialRxNotifierThread.signal_set(SIGNAL_SERIAL_RX);

	char receivedChar = serial->getc();

	if (receivedChar == '\n') {
		commandManager->handleCommand(commandBuffer);
		commandBuffer = "";
	} else {
		commandBuffer += receivedChar;
	}
}

void Application::handleSerialTx() {
	//serialTxNotifierThread.signal_set(SIGNAL_SERIAL_TX);
}
