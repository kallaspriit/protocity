#ifndef SOCKETSERVER_HPP
#define SOCKETSERVER_HPP

#include "mbed.h"

class EthernetInterface;

class SocketServer {

public:
	class MessageListener {
	public:
		void onSocketMessageReceived(char *message, int size);
	};

	bool start(EthernetInterface *ethernetInterface, int port = 8080);

private:
	EthernetInterface *ethernetInterface = NULL;

	const int SOCKET_RECEIVE_TIMEOUT_MS = 5000;
};

#endif
