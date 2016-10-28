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
		virtual void onSocketCommandReceived(const char *command, int length) = 0;
	};

	bool start(EthernetInterface *ethernetInterface, int port = 8080);
	bool isClientConnected();
	TCPSocketConnection *getConnectedClient();
	bool sendMessage(std::string message);
	bool sendMessage(char *message, int length);

	void addListener(SocketServerListener *socketServerListener);

private:
	void runListenThread();

	TCPSocketServer tpcSocketServer;
	TCPSocketConnection *connectedClient = NULL;

	Thread listenThread;

	static const int MAX_COMMAND_LENGTH = 64;
	static const int SEND_BUFFER_SIZE = 64;
	char commandBuffer[MAX_COMMAND_LENGTH + 1];
	char sendBuffer[SEND_BUFFER_SIZE];
	int commandLength = 0;

	std::vector<SocketServerListener*> listeners;

	const int SOCKET_RECEIVE_TIMEOUT_MS = 5000;
};

#endif
