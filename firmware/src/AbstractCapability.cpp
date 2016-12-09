#include "AbstractCapability.hpp"

AbstractCapability::AbstractCapability(Serial *serial, PortController *portController) :
	serial(serial),
	portController(portController)
{}
