#ifndef ETHERNETMANAGER_HPP
#define ETHERNETMANAGER_HPP

#include "mbed.h"
#include "EthernetInterface.h"

class EthernetManager {

public:
	bool initialize();

	EthernetInterface *getEthernetInterface();

private:
	EthernetInterface ethernetInterface;
};

#endif
