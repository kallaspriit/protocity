#include "AbstractCapability.hpp"

AbstractCapability::AbstractCapability(Serial *serial, PortController *portController) :
	serial(serial),
	portController(portController)
{}

/*
void AbstractCapability::startStepThread() {
	thread.start(this, &AbstractCapability::runThread);
}

void AbstractCapability::runThread() {
	timer.start();

	while (true) {
		int deltaUs = timer.read_us();
		timer.reset();

		step(deltaUs);
	}
}

void AbstractCapability::step(int deltaUs) {
	printf("update capability..\n");

	Thread::wait(1000);
}
*/
