#include "mbed.h"

#include "CommandManager.hpp"

#include <string>

void CommandManager::Command::reset() {
	name = "";

	for (int i = 0; i < MAX_ARGUMENT_COUNT; i++) {
		arguments[i] = "";
	}

	argumentCount = 0;
}

std::string CommandManager::Command::getString(int argumentIndex){
	validateArgumentIndex(argumentIndex);

	return arguments[argumentIndex];
}

int CommandManager::Command::getInt(int argumentIndex){
	validateArgumentIndex(argumentIndex);

	return atoi(arguments[argumentIndex].c_str());
}

float CommandManager::Command::getFloat(int argumentIndex){
	validateArgumentIndex(argumentIndex);

	return atof(arguments[argumentIndex].c_str());
}

double CommandManager::Command::getDouble(int argumentIndex) {
	return (double)getFloat(argumentIndex);
}

void CommandManager::Command::validateArgumentIndex(int argumentIndex) {
	if (argumentIndex > argumentCount - 1) {
		error("requested argument with index #%d but there are only %d arguments provided\n", argumentIndex, argumentCount);
	}
}

void CommandManager::handleCommand(const char *commandText, int length) {
	printf("> got string command: '%s'\n", commandText);

	if ((commandQueueTail - commandQueueHead) == COMMAND_QUEUE_SIZE) {
		error("command queue fits a maximum of %d commands\n", COMMAND_QUEUE_SIZE);
	}

	Command *command = &commandQueue[commandQueueTail % COMMAND_QUEUE_SIZE];
	command->reset();

	commandQueueTail++;

	bool isFirstDelimiter = true;

	// handle commands like "led:1:ON"
	for (int i = 0; i < length; i++) {
		char character = commandText[i];

		if (character == ':') {
			// handle delimiter
			if (isFirstDelimiter) {
				command->name = commandNameBuffer;

				isFirstDelimiter = false;
				commandNameBuffer = "";
			} else {
				if (command->argumentCount == Command::MAX_ARGUMENT_COUNT) {
					error("command can have a maximum of %d arguments", Command::MAX_ARGUMENT_COUNT);
				}

				command->arguments[command->argumentCount++] = argumentBuffer;

				argumentBuffer = "";
			}
		} else {
			// handle other characters
			if (isFirstDelimiter) {
				commandNameBuffer += character;
			} else {
				argumentBuffer += character;
			}
		}
	}

	// handle no delimiters
	if (isFirstDelimiter && commandNameBuffer.size() > 0) {
		command->name = commandNameBuffer;
	}

	// add last argument
	if (argumentBuffer.size() > 0) {
		command->arguments[command->argumentCount++] = argumentBuffer;
	}

	// reset
	commandNameBuffer = "";
	argumentBuffer = "";
}

int CommandManager::getQueuedCommandCount() {
	return commandQueueTail - commandQueueHead;
}

CommandManager::Command *CommandManager::getNextCommand() {
	if (commandQueueHead == commandQueueTail) {
		return NULL;
	}

	Command *command = &commandQueue[commandQueueHead % COMMAND_QUEUE_SIZE];

	commandQueueHead++;

	return command;
}
