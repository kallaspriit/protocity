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

	// select command from the ring-buffer
	Command *command = &commandQueue[commandQueueTail % COMMAND_QUEUE_SIZE];
	command->reset();

	// keep count of how many ":" delimiters we have seen
	int delimiterCount = 0;

	// handle commands like ID:NAME:arg1:arg2:argN, for example 1:led:1:ON
	for (int i = 0; i < length; i++) {
		char character = commandText[i];

		// handle delimiter
		if (character == ':') {
			delimiterCount++;

			// handle id, name and arguments
			if (delimiterCount == 1) {
				command->id = atoi(commandIdBuffer.c_str());
				commandIdBuffer = "";
			} else if (delimiterCount == 2) {
				command->name = commandNameBuffer;
				commandNameBuffer = "";
			} else {
				if (command->argumentCount == Command::MAX_ARGUMENT_COUNT) {
					printf("> command can have a maximum of %d arguments\n", Command::MAX_ARGUMENT_COUNT);

					return;
				}

				command->arguments[command->argumentCount++] = argumentBuffer;

				argumentBuffer = "";
			}
		} else {
			// handle other characters
			if (delimiterCount == 0) {
				commandIdBuffer += character;
			} else if (delimiterCount == 1) {
				commandNameBuffer += character;
			} else {
				argumentBuffer += character;
			}
		}
	}

	if (delimiterCount == 0) {
		printf("> expected commands in the format of ID:NAME:arg1:arg2:argN\n");

		return;
	}

	// handle no delimiters
	if (delimiterCount == 1 && commandNameBuffer.size() > 0) {
		command->name = commandNameBuffer;
	}

	// add last argument
	if (argumentBuffer.size() > 0) {
		if (command->argumentCount == Command::MAX_ARGUMENT_COUNT) {
			printf("> command can have a maximum of %d arguments\n", Command::MAX_ARGUMENT_COUNT);

			return;
		}

		command->arguments[command->argumentCount++] = argumentBuffer;
	}

	// reset
	commandIdBuffer = "";
	commandNameBuffer = "";
	argumentBuffer = "";

	// increment command count
	commandQueueTail++;
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
