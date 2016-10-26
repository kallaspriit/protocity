#ifndef ETHERNETMANAGER_HPP
#define ETHERNETMANAGER_HPP

#include "mbed.h"

class EthernetInterface;

class EthernetManager {

public:
	bool initialize();

	EthernetInterface *getEthernetInterface();

private:
	EthernetInterface *ethernetInterface = NULL;
};

#endif
