#ifndef SOCKETSERVER_HPP
#define SOCKETSERVER_HPP

#include "mbed.h"

#include <vector>
#include <string>

class EthernetInterface;
class TCPSocketConnection;

class SocketServer {

public:
	class MessageListener {
	public:
		virtual void onSocketMessageReceived(std::string message) = 0;
	};

	bool start(EthernetInterface *ethernetInterface, int port = 8080);
	bool isClientConnected();
	TCPSocketConnection *getConnectedClient();
	bool sendMessage(std::string message);

	void addMessageListener(MessageListener *messageListener);

private:
	EthernetInterface *ethernetInterface = NULL;
	TCPSocketConnection *connectedClient = NULL;

	std::string messageBuffer;

	std::vector<MessageListener*> messageListeners;

	const int SOCKET_RECEIVE_TIMEOUT_MS = 5000;
};

#endif
