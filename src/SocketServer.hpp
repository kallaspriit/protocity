#ifndef SOCKETSERVER_HPP
#define SOCKETSERVER_HPP

#include "mbed.h"
#include "rtos.h"
#include "EthernetInterface.h"

#include <vector>
#include <string>

class SocketServer {

public:
	class SocketServerListener {
	public:
		virtual void onSocketClientConnected(TCPSocketConnection* client) = 0;
		virtual void onSocketClientDisconnected(TCPSocketConnection* client) = 0;
		virtual void onSocketMessageReceived(std::string message) = 0;
	};

	bool start(EthernetInterface *ethernetInterface, int port = 8080);
	bool isClientConnected();
	TCPSocketConnection *getConnectedClient();
	bool sendMessage(std::string message);

	void addListener(SocketServerListener *socketServerListener);

private:
	void runListenThread();

	TCPSocketServer tpcSocketServer;
	TCPSocketConnection *connectedClient = NULL;

	Thread listenThread;
	std::string messageBuffer;

	std::vector<SocketServerListener*> listeners;

	const int SOCKET_RECEIVE_TIMEOUT_MS = 5000;
};

#endif
