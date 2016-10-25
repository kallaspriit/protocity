#ifndef COMMANDMANAGER_HPP
#define COMMANDMANAGER_HPP

#include <string>

class CommandManager {

public:
	void handleCommand(std::string command);

private:
	void handleJsonCommand(std::string command);
	void handleStringCommand(std::string command);
	bool isJsonCommand(std::string command);
};

#endif
