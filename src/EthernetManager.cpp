#include "EthernetManager.hpp"

#include "EthernetInterface.h"

void EthernetManager::initialize() {
	ethernetInterface = new EthernetInterface();

	printf("> initializing ethernet interface..\n");

    int initResult = ethernetInterface->init(); // use DHCP
    // int initResult = eth.init("10.220.20.123", "255.255.255.0", "10.220.20.1"); // use static IP

	if (initResult == 0) {
		printf("  success!\n");
	} else {
		printf("  failed with code %d\n", initResult);

		return;
	}

	printf("> connecting to ethernet..\n");

    int connectResult = ethernetInterface->connect();

	if (connectResult == 0) {
		printf("  success, ip address: %s\n", ethernetInterface->getIPAddress());
	} else {
		printf("  failed with code %d\n", connectResult);

		return;
	}

    TCPSocketConnection sock;
    sock.connect("mbed.org", 80);

    char http_cmd[] = "GET /media/uploads/mbed_official/hello.txt HTTP/1.0\n\n";
    sock.send_all(http_cmd, sizeof(http_cmd)-1);

    char buffer[300];
    int ret;
    while (true) {
        ret = sock.receive(buffer, sizeof(buffer)-1);
        if (ret <= 0)
            break;
        buffer[ret] = '\0';
        printf("Received %d chars from server:\n%s\n", ret, buffer);
    }

    sock.close();

    ethernetInterface->disconnect();
}
