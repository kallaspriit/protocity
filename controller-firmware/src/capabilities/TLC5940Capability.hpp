#ifndef TLC5940CAPABILITY_HPP
#define TLC5940CAPABILITY_HPP

#include "../AbstractCapability.hpp"

#include <TLC5940.hpp>

#include <string>

class TLC5940Capability : public AbstractCapability {

public:
	TLC5940Capability(Serial *serial, PortController *portController, PinName mosiPin, PinName sclkPin, PinName blankPin, PinName vprgPin, PinName gsclkPin, int chainLength = 1);

	std::string getName();
	CommandManager::Command::Response execute(CommandManager::Command *command);
	CommandManager::Command::Response handleValueCommand(CommandManager::Command *command);
	void update(int deltaUs);

private:
	bool enable();
	void disable();

	const PinName mosiPin;
	const PinName sclkPin;
	const PinName blankPin;
	const PinName vprgPin;
	const PinName gsclkPin;
	const int chainLength;

	TLC5940 *tlc5940 = NULL;
	bool isEnabled = false;
	std::string activeTagName = "";

	static const int SEND_BUFFER_SIZE = 64;
	char sendBuffer[SEND_BUFFER_SIZE];

	unsigned short *values;
};

#endif
