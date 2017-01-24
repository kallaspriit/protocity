#ifndef TMP102CAPABILITY_HPP
#define TMP102CAPABILITY_HPP

#include "../AbstractCapability.hpp"

#include <TMP102.hpp>

// https://www.sparkfun.com/products/11931
class TMP102Capability : public AbstractCapability {

public:
	TMP102Capability(Serial *serial, PortController *portController);

	std::string getName();
	CommandManager::Command::Response execute(CommandManager::Command *command);
	void update(int deltaUs);

private:
	void enable();
	void disable();
	void sendMeasurement();

	Timer timer;
	TMP102 *sensor = NULL;
	bool isEnabled = false;
	int measurementIntervalMs = 5000;
};

#endif
