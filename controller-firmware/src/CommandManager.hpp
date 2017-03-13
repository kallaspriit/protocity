#ifndef COMMANDMANAGER_HPP
#define COMMANDMANAGER_HPP

#include "Log.hpp"

#include <string>

class CommandManager {

public:
	class Command {
	public:
		class Response {

		public:
			Response(int requestId);

			static const int MAX_ARGUMENT_COUNT = 8;
			static const int RESPONSE_BUFFER_SIZE = 1024;

			int requestId;
			int argumentCount = 0;
			std::string arguments[MAX_ARGUMENT_COUNT];
			std::string errorMessage = "";

			void addArgument(std::string argument);
			std::string getResponseText();

		private:
			Log log = Log::getLog("CommandManager::Command::Response");

			std::string getSuccessResponseText();
			std::string getErrorResponseText();

			char *responseBuffer;
		};

		static const int MAX_ARGUMENT_COUNT = 8;

		int sourceId = 0;
		int id = 0;
		int argumentCount = 0;
		std::string name = "";
		std::string arguments[MAX_ARGUMENT_COUNT];

		void reset();

		std::string getString(int argumentIndex);
		int getInt(int argumentIndex);
		float getFloat(int argumentIndex);
		double getDouble(int argumentIndex);

		CommandManager::Command::Response createSuccessResponse();
		CommandManager::Command::Response createSuccessResponse(int value);
		CommandManager::Command::Response createSuccessResponse(int value1, int value2);
		CommandManager::Command::Response createSuccessResponse(int value1, int value2, int value3);
		CommandManager::Command::Response createSuccessResponse(float value);
		CommandManager::Command::Response createSuccessResponse(std::string message);
		CommandManager::Command::Response createFailureResponse();
		CommandManager::Command::Response createFailureResponse(std::string errorMessage);

	private:
		Log log = Log::getLog("CommandManager::Command");

		void validateArgumentIndex(int argumentIndex);
	};

	CommandManager();

	void handleCommand(int sourceId, const char *command, int length);
	int getQueuedCommandCount();
	CommandManager::Command *getNextCommand();

private:
	Log log = Log::getLog("CommandManager");

	void reset();

	static const int COMMAND_QUEUE_SIZE = 32;

	Command *commandQueue;
	int commandQueueTail = 0;
	int commandQueueHead = 0;
	std::string commandIdBuffer;
	std::string commandNameBuffer;
	std::string argumentBuffer;

};

#endif
