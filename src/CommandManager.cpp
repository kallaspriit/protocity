#include "mbed.h"
#include <picojson.hpp>

#include "CommandManager.hpp"

bool CommandManager::isJsonCommand(const char *command, int length) {
	return length >= 2 && command[0] == '{' && command[length - 1] == '}';
}

void CommandManager::handleJsonCommand(const char *command, int length) {
	printf("> got JSON command: '%s'\n", command);

	picojson::value info;
	std::string parseError = picojson::parse(info, command, command + strlen(command));
	// std::string parseError = picojson::parse(info, command, command + length);

	if (!parseError.empty()) {
		printf("> parsing command '%s' as json failed (%s)\n", command, parseError.c_str());

		return;
	}

	int id = (int)info.get("id").get<double>();

	printf("> command id: %d\n", id);
}

void CommandManager::handleStringCommand(const char *command, int length) {
	printf("> got string command: '%s'\n", command);
}

void CommandManager::handleCommand(const char *command, int length) {
	if (isJsonCommand(command, length)) {
		handleJsonCommand(command, length);
	} else {
		handleStringCommand(command, length);
	}
}
