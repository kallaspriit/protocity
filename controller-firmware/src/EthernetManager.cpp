#include "EthernetManager.hpp"

bool EthernetManager::initialize() {
	printf("# initializing ethernet interface..\n");

    int initResult = ethernetInterface.init(); // use DHCP
    // int initResult = eth.init("10.220.20.123", "255.255.255.0", "10.220.20.1"); // use static IP

	if (initResult == 0) {
		printf("# ethernet interface initialized\n");
	} else {
		printf("# failed to initialize ethernet interface with code %d\n", initResult);

		return false;
	}

	printf("# connecting to ethernet..\n");

    int connectResult = ethernetInterface.connect();

	if (connectResult == 0) {
		printf("# connected to ethernet, ip address: %s\n", ethernetInterface.getIPAddress());
	} else {
		printf("# failed to connect to ethernet with code %d\n", connectResult);

		return false;
	}

	return true;
}

EthernetInterface *EthernetManager::getEthernetInterface() {
	return &ethernetInterface;
}
