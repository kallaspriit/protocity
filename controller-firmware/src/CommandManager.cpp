#include "mbed.h"

#include "CommandManager.hpp"

#include <string>
#include <sstream>

void CommandManager::Command::reset() {
	name = "";

	for (int i = 0; i < MAX_COMMAND_ARGUMENT_COUNT; i++) {
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
		log.error("requested argument with index #%d but there are only %d arguments provided\n", argumentIndex, argumentCount);
	}
}

std::string CommandManager::Command::Response::getResponseText() {
	if (errorMessage.length() == 0) {
		return getSuccessResponseText();
	} else {
		return getErrorResponseText();
	}
}

std::string CommandManager::Command::Response::getSuccessResponseText() {
	std::ostringstream ss;

	ss << requestId << ":OK";

	for (int i = 0; i < argumentCount; i++) {
		ss << ":" << arguments[i];
	}

	return ss.str();
}

std::string CommandManager::Command::Response::getErrorResponseText() {
	std::ostringstream ss;

	ss << requestId << ":ERROR:" << errorMessage;
}

void CommandManager::Command::Response::reset(int requestId) {
	this->requestId = requestId;
	this->argumentCount = 0;
	this->errorMessage = "";
}

void CommandManager::Command::Response::addString(std::string argument) {
	if (argumentCount == MAX_RESPONSE_ARGUMENT_COUNT) {
		log.warn("response can have a maximum of %d arguments\n", MAX_RESPONSE_ARGUMENT_COUNT);

		return;
	}

	arguments[argumentCount++] = argument;
}

void CommandManager::Command::Response::addInt(int argument) {
	std::ostringstream ss;

	ss << argument;

	addString(ss.str());
}

void CommandManager::Command::Response::addFloat(float argument) {
	std::ostringstream ss;

	ss << argument;

	addString(ss.str());
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse() {
	response.reset(id);

	return response;
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse(int value) {
	response.reset(id);
	response.addInt(value);

	return response;
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse(int value1, int value2) {
	response.reset(id);
	response.addInt(value1);
	response.addInt(value2);

	return response;
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse(int value1, int value2, int value3) {
	response.reset(id);
	response.addInt(value1);
	response.addInt(value2);
	response.addInt(value3);

	return response;
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse(float value) {
	response.reset(id);
	response.addFloat(value);

	return response;
}

CommandManager::Command::Response CommandManager::Command::createSuccessResponse(std::string message) {
	response.reset(id);
	response.addString(message);

	return response;
}

CommandManager::Command::Response CommandManager::Command::createFailureResponse() {
	return createFailureResponse("error occured");
}

CommandManager::Command::Response CommandManager::Command::createFailureResponse(std::string errorMessage) {
	response.reset(id);
	response.errorMessage = errorMessage;

	return response;
}

CommandManager::CommandManager() {
	commandQueue = new Command[COMMAND_QUEUE_SIZE];
}

void CommandManager::handleCommand(int sourceId, const char *commandText, int length) {
	log.debug("< %s", commandText);

	if ((commandQueueTail - commandQueueHead) == COMMAND_QUEUE_SIZE) {
		log.warn("command queue fits a maximum of %d commands", COMMAND_QUEUE_SIZE);

		return;
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
				if (command->argumentCount == Command::MAX_COMMAND_ARGUMENT_COUNT) {
					log.warn("command can have a maximum of %d arguments", Command::MAX_COMMAND_ARGUMENT_COUNT);

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
		log.warn("expected commands in the format of ID:NAME:arg1:arg2:argN");

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
		if (command->argumentCount == Command::MAX_COMMAND_ARGUMENT_COUNT) {
			log.warn("command can have a maximum of %d arguments", Command::MAX_COMMAND_ARGUMENT_COUNT);

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
