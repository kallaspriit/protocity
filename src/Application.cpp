#include "Application.hpp"
#include "Config.hpp"

#include <stdio.h>
#include <stdlib.h>
#define FREEMEM_CELL 10
struct elem { /* Definition of a structure that is FREEMEM_CELL bytes  in size.) */
    struct elem *next;
    char dummy[FREEMEM_CELL-2];
};
int getFreeMemory(void) {
    int counter;
    struct elem *head, *current, *nextone;
    current = head = (struct elem*) malloc(sizeof(struct elem));
    if (head == NULL)
        return 0;      /*No memory available.*/
    counter = 0;
   // __disable_irq();
    do {
        counter++;
        current->next = (struct elem*) malloc(sizeof(struct elem));
        current = current->next;
    } while (current != NULL);
    /* Now counter holds the number of type elem
       structures we were able to allocate. We
       must free them all before returning. */
    current = head;
    do {
        nextone = current->next;
        free(current);
        current = nextone;
    } while (nextone != NULL);
   // __enable_irq();

    return counter*FREEMEM_CELL;
}

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
	int freeMemoryBytes = getFreeMemory();
	int queuedCommandCount = commandManager.getQueuedCommandCount();

	printf("> main loop %f, free memory: %d, queued commands: %d\n", timer.read(), freeMemoryBytes, queuedCommandCount);

	CommandManager::Command *command = commandManager.getNextCommand();

	while (command != NULL) {
		printf("  handle command '%s' with %d arguments\n", command->name.c_str(), command->argumentCount);

		for (int i = 0; i < command->argumentCount; i++) {
			printf("    argument %d: %s\n", i, command->arguments[i].c_str());
		}

		if (command->name == "led") {
			printf("> calling led callback\n");

			_function.call(command->argumentCount, command->arguments);
		}

		command = commandManager.getNextCommand();
	}

	Thread::wait(10000);
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
	registerCommandHandler("led", 2, this, &Application::handleLedCommand);
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
void Application::registerCommandHandler(std::string name, int argumentCount, T *obj, M method) {
	registerCommandHandler(name, argumentCount, Callback<void(int, std::string[])>(obj, method));
}

void Application::registerCommandHandler(std::string name, int argumentCount, Callback<void(int, std::string[])> func) {
	_function.attach(func);

	printf("> registering command handler for '%s' with %d arguments\n", name.c_str(), argumentCount);
}

void Application::onSocketClientConnected(TCPSocketConnection* client) {
	debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::ON);
}

void Application::onSocketClientDisconnected(TCPSocketConnection* client) {
	debug.setLedMode(LED_ETHERNET_STATUS_INDEX, Debug::LedMode::BLINK_SLOW);
}

void Application::onSocketCommandReceived(const char *command, int length) {
	commandManager.handleCommand(command, length);

	debug.setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);
}

void Application::handleSerialRx() {
	char receivedChar = serial.getc();

	if (receivedChar == '\n') {
		commandManager.handleCommand(commandBuffer, commandLength);

		commandBuffer[0] = '\0';
		commandLength = 0;

		debug.setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);
	} else {
		if (commandLength > MAX_COMMAND_LENGTH - 1) {
			error("maximum command length is %d characters, stopping at %s\n", MAX_COMMAND_LENGTH, commandBuffer);
		}

		commandBuffer[commandLength++] = receivedChar;
		commandBuffer[commandLength] = '\0';
	}
}

void Application::handleLedCommand(int argumentCount, std::string arguments[]) {
	printf("> handling led command with %d arguments\n", argumentCount);

	for (int i = 0; i < argumentCount; i++) {
		printf("    argument %d: %s\n", i, arguments[i].c_str());
	}
}
