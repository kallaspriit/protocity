#ifndef SOCKETSERVER_HPP
#define SOCKETSERVER_HPP

#include "Log.hpp"
#include "EthernetInterface.h"

#include "mbed.h"
#include "rtos.h"

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

	SocketServer();

	bool start(EthernetInterface *ethernetInterface, int port = 8080);
	bool isClientConnected();
	TCPSocketConnection *getConnectedClient();
	void dropConnection();

	bool sendMessage(std::string message);
	bool sendMessage(char *message, int length);

	void addListener(SocketServerListener *listener);

private:
	Log log = Log::getLog("SocketServer");

	void runListenThread();
	void handleReceivedData(char *buffer, int receivedBytes);

	TCPSocketServer tpcSocketServer;
	TCPSocketConnection *connectedClient = NULL;

	Thread listenThread;

	static const int MAX_COMMAND_LENGTH = 1024;
	static const int SEND_BUFFER_SIZE = 1024;
	static const int RECEIVE_BUFFER_SIZE = 1024;
	static const int COMMAND_BUFFER_SIZE = MAX_COMMAND_LENGTH + 1;
	char *commandBuffer;
	char *receiveBuffer;
	char *sendBuffer;
	int commandLength = 0;

	typedef std::vector<SocketServerListener*> ListenerList;
	ListenerList listeners;

	const int SOCKET_RECEIVE_TIMEOUT_MS = 5000;
};

#endif
