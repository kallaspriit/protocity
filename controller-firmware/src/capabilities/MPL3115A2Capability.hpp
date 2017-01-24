#ifndef MPL3115A2CAPABILITY_HPP
#define MPL3115A2CAPABILITY_HPP

#include "../AbstractCapability.hpp"

#include <MPL3115A2.hpp>

// https://www.sparkfun.com/products/11084
class MPL3115A2Capability : public AbstractCapability {

public:
	MPL3115A2Capability(Serial *serial, PortController *portController);

	std::string getName();
	CommandManager::Command::Response execute(CommandManager::Command *command);
	void update(int deltaUs);

private:
	void enable();
	void disable();
	void sendMeasurement();

	Timer timer;
	MPL3115A2 *sensor = NULL;
	bool isEnabled = false;
	int measurementIntervalMs = 5000;
};

#endif
