#include "mbed.h"

#include "CommandManager.hpp"

bool CommandManager::isJsonCommand(std::string command) {
	return command.size() >= 2 && command[0] == '{' && command[command.size() - 1] == '}';
}

void CommandManager::handleJsonCommand(std::string command) {
	printf("> got JSON command: '%s'\n", command.c_str());
}

void CommandManager::handleStringCommand(std::string command) {
	printf("> got string command: '%s'\n", command.c_str());
}

void CommandManager::handleCommand(std::string command) {
	if (isJsonCommand(command)) {
		handleJsonCommand(command);
	} else {
		handleStringCommand(command);
	}
}
