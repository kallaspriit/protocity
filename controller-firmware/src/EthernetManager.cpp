#include "EthernetManager.hpp"

bool EthernetManager::initialize() {
	log.info("initializing ethernet interface..");

    int initResult = ethernetInterface.init(); // use DHCP
    // int initResult = eth.init("10.220.20.123", "255.255.255.0", "10.220.20.1"); // use static IP

	if (initResult == 0) {
		log.info("ethernet interface initialized");
	} else {
		log.warn("failed to initialize ethernet interface with code %d", initResult);

		return false;
	}

	log.info("connecting to ethernet..");

    int connectResult = ethernetInterface.connect();

	if (connectResult == 0) {
		log.info("connected to ethernet, ip address: %s", ethernetInterface.getIPAddress());
	} else {
		log.warn("failed to connect to ethernet with code %d", connectResult);

		return false;
	}

	return true;
}

EthernetInterface *EthernetManager::getEthernetInterface() {
	return &ethernetInterface;
}
