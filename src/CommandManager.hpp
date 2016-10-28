#ifndef COMMANDMANAGER_HPP
#define COMMANDMANAGER_HPP

#include <string>

class CommandManager {

public:
	class Command {
	public:
		static const int MAX_ARGUMENT_COUNT = 8;

		std::string name = "";
		std::string arguments[MAX_ARGUMENT_COUNT];
		int argumentCount = 0;

		void reset();

		std::string getString(int argumentIndex);
		int getInt(int argumentIndex);
		float getFloat(int argumentIndex);
		double getDouble(int argumentIndex);

	private:
		void validateArgumentIndex(int argumentIndex);
	};

	void handleCommand(const char *command, int length);
	int getQueuedCommandCount();
	CommandManager::Command *getNextCommand();

private:
	static const int COMMAND_QUEUE_SIZE = 8;

	Command commandQueue[COMMAND_QUEUE_SIZE];
	int commandQueueTail = 0;
	int commandQueueHead = 0;
	std::string commandNameBuffer;
	std::string argumentBuffer;

};

#endif
