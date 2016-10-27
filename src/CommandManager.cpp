#include "mbed.h"
#include <picojson.hpp>

#include "CommandManager.hpp"

bool CommandManager::isJsonCommand(std::string command) {
	return command.size() >= 2 && command[0] == '{' && command[command.size() - 1] == '}';
}

void CommandManager::handleJsonCommand(std::string command) {
	printf("> got JSON command: '%s'\n", command.c_str());

	picojson::value info;
	const char *json = command.c_str();
	std::string parseError = picojson::parse(info, json, json + strlen(json));

	if (!parseError.empty()) {
		printf("> parsing command '%s' as json failed (%s)\n", command.c_str(), parseError.c_str());

		return;
	}

	int id = (int)info.get("id").get<double>();

	printf("> command id: %d\n", id);
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
