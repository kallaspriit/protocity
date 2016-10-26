#ifndef ETHERNETMANAGER_HPP
#define ETHERNETMANAGER_HPP

#include "mbed.h"

class EthernetInterface;

class EthernetManager {

public:
	void initialize();

private:
	EthernetInterface *ethernetInterface = NULL;
};

#endif
