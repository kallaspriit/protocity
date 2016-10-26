#include "EthernetManager.hpp"

#include "EthernetInterface.h"

bool EthernetManager::initialize() {
	ethernetInterface = new EthernetInterface();

	printf("> initializing ethernet interface..\n");

    int initResult = ethernetInterface->init(); // use DHCP
    // int initResult = eth.init("10.220.20.123", "255.255.255.0", "10.220.20.1"); // use static IP

	if (initResult == 0) {
		printf("  success\n");
	} else {
		printf("  failed with code %d\n", initResult);

		return false;
	}

	printf("> connecting to ethernet..\n");

    int connectResult = ethernetInterface->connect();

	if (connectResult == 0) {
		printf("  success, ip address: %s\n", ethernetInterface->getIPAddress());
	} else {
		printf("  failed with code %d\n", connectResult);

		return false;
	}

	return true;
}

EthernetInterface *EthernetManager::getEthernetInterface() {
	return ethernetInterface;
}
