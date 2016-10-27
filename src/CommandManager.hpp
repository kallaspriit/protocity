#ifndef COMMANDMANAGER_HPP
#define COMMANDMANAGER_HPP

#include <string>

class CommandManager {

public:
	void handleCommand(const char *command, int length);

private:
	void handleJsonCommand(const char *command, int length);
	void handleStringCommand(const char *command, int length);
	bool isJsonCommand(const char *command, int length);
};

#endif
