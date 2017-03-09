#ifndef ETHERNETMANAGER_HPP
#define ETHERNETMANAGER_HPP

#include "Log.hpp"

#include "mbed.h"
#include "EthernetInterface.h"

class EthernetManager {

public:
	bool initialize();

	EthernetInterface *getEthernetInterface();

private:
	Log log = Log::getLog("EthernetManager");

	EthernetInterface ethernetInterface;
};

#endif
