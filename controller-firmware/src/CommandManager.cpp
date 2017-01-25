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

CommandManager::Command::Response::Response(int requestId) {
	this->requestId = requestId;

	responseBuffer = new char[RESPONSE_BUFFER_SIZE];
}

std::string CommandManager::Command::Response::getResponseText() {
	if (errorMessage.length() == 0) {
		return getSuccessResponseText();
	} else {
		return getErrorResponseText();
	}
}

std::string CommandManager::Command::Response::getSuccessResponseText() {
	snprintf(responseBuffer, RESPONSE_BUFFER_SIZE, "%d:OK", requestId);

	std::string responseText = std::string(responseBuffer);

	for (int i = 0; i < argumentCount; i++) {
		responseText += ":" + arguments[i];
	}

	return responseText;
}

std::string CommandManager::Command::Response::getErrorResponseText() {
	snprintf(responseBuffer, RESPONSE_BUFFER_SIZE, "%d:ERROR:%s", requestId, errorMessage.c_str());

	return std::string(responseBuffer);
}

void CommandManager::Command::Response::addArgument(std::string argument) {
	if (argumentCount == MAX_ARGUMENT_COUNT) {
		error("response can have a maximum of %d arguments\n", MAX_ARGUMENT_COUNT);
	}

	arguments[argumentCount++] = argument;
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse(/*const char* fmt...*/) {
	return CommandManager::Command::Response(id);
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse(int value) {
	CommandManager::Command::Response response(id);

	char buf[10];
	sprintf(buf, "%d", value);

	response.addArgument(buf);

	return response;
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse(float value) {
	CommandManager::Command::Response response(id);

	char buf[10];
	sprintf(buf, "%f", value);

	response.addArgument(buf);

	return response;
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse(std::string message) {
	CommandManager::Command::Response response(id);

	response.addArgument(message);

	return response;
}

CommandManager::Command::Response CommandManager::Command::createFailureResponse() {
	return createFailureResponse("error occured");
}

CommandManager::Command::Response CommandManager::Command::createFailureResponse(std::string errorMessage) {
	CommandManager::Command::Response response(id);

	response.errorMessage = errorMessage;

	return response;
}

CommandManager::CommandManager() {
	commandQueue = new Command[COMMAND_QUEUE_SIZE];
}

void CommandManager::handleCommand(int sourceId, const char *commandText, int length) {
	printf("< %s\n", commandText);

	if ((commandQueueTail - commandQueueHead) == COMMAND_QUEUE_SIZE) {
		error("command queue fits a maximum of %d commands\n", COMMAND_QUEUE_SIZE);
	}

	// select command from the ring-buffer
	Command *command = &commandQueue[commandQueueTail % COMMAND_QUEUE_SIZE];
	command->reset();
	command->sourceId = sourceId;

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
					printf("# command can have a maximum of %d arguments\n", Command::MAX_ARGUMENT_COUNT);

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
		printf("# expected commands in the format of ID:NAME:arg1:arg2:argN\n");

		command->id = 0;
		command->name = "";

		reset();

		// increment command count
		commandQueueTail++;

		return;
	}

	// handle no delimiters
	if (delimiterCount == 1 && commandNameBuffer.size() > 0) {
		command->name = commandNameBuffer;
	}

	// add last argument
	if (argumentBuffer.size() > 0) {
		if (command->argumentCount == Command::MAX_ARGUMENT_COUNT) {
			printf("# command can have a maximum of %d arguments\n", Command::MAX_ARGUMENT_COUNT);

			return;
		}

		command->arguments[command->argumentCount++] = argumentBuffer;
	}

	reset();

	// increment command count
	commandQueueTail++;
}

void CommandManager::reset() {
	commandIdBuffer = "";
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
