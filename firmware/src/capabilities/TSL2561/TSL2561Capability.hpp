#ifndef TSL2561CAPABILITY_HPP
#define TSL2561CAPABILITY_HPP

#include "../../AbstractCapability.hpp"
#include "../../libs/TSL2561/TSL2561.hpp"

class TSL2561Capability : public AbstractCapability {

public:
	TSL2561Capability(PortController *portController);

	std::string getName();
	CommandManager::Command::Response execute(CommandManager::Command *command);
	void update(int deltaUs);

private:
	void enable();
	void disable();
	void sendMeasurement();

	Timer timer;
	TSL2561 *sensor = NULL;
	bool isEnabled = false;
	int measurementIntervalMs = 5000;

	static const int SEND_BUFFER_SIZE = 32;
	char sendBuffer[SEND_BUFFER_SIZE];
};

#endif
