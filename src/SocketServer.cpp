#include "SocketServer.hpp"

#include "EthernetInterface.h"

bool SocketServer::start(EthernetInterface *ethernetInterface, int port) {
	tpcSocketServer = new TCPSocketServer();

	printf("> starting socket server on port %d\n", port);

    int bindResult = tpcSocketServer->bind(port);

	if (bindResult == 0) {
		printf("  binding to port %d was successful\n", port);
	} else {
		printf("  binding to port %d failed\n", port);

		return false;
	}

    int listenResult = tpcSocketServer->listen();

	if (listenResult == 0) {
		printf("  listening on port %d was successful\n", port);
	} else {
		printf("  listening on port %d failed\n", port);

		return false;
	}

	listenThread.start(this, &SocketServer::runListenThread);

	return true;
}

void SocketServer::runListenThread() {
	printf("> starting socket server listen thread");

	while (true) {
        printf("> waiting for new connection...\n");

        TCPSocketConnection client;
        int acceptResult = tpcSocketServer->accept(client);

		if (acceptResult != 0) {
			printf("> accepting new client failed\n");

			break;
		}

        client.set_blocking(false, SOCKET_RECEIVE_TIMEOUT_MS);

        printf("> got socket connection from: %s\n", client.get_address());

		connectedClient = &client;

		// notify listeners
		for (std::vector<SocketServerListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
			(*it)->onSocketClientConnected(connectedClient);
		}

        char buffer[256];

        while (true) {
			// check whether the connection is still valid
			if (!client.is_connected()) {
				printf("> socket connection to %s has been closed\n", client.get_address());

				// notify listeners
				for (std::vector<SocketServerListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
					(*it)->onSocketClientDisconnected(connectedClient);
				}

				client.close();
				connectedClient = NULL;

				break;
			}

			// attempt to receive some data
            int receivedBytes = client.receive(buffer, sizeof(buffer));

			// just try again if nothing received
            if (receivedBytes <= 0) {
				continue;
			}

			// search for newline delimiting commands
			for (int i = 0; i < receivedBytes; i++) {
				char receivedChar = buffer[i];

				if (receivedChar == '\n') {
					// only display the message if no listeners have been registered
					if (listeners.size() == 0) {
						printf("> received command: '%s'\n", messageBuffer.c_str());
					}

					// notify all message listeners
					for (std::vector<SocketServerListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
						(*it)->onSocketMessageReceived(messageBuffer);
					}

					// send response
					// sendMessage("got command '" + messageBuffer + "'");

					messageBuffer = "";
				} else {
					messageBuffer += receivedChar;
				}
			}
        }
    }
}

bool SocketServer::isClientConnected() {
	return connectedClient != NULL && connectedClient->is_connected();
}

TCPSocketConnection *SocketServer::getConnectedClient() {
	return connectedClient;
}

bool SocketServer::sendMessage(std::string message) {
	if (!isClientConnected()) {
		return false;
	}

	char *messageBuffer = new char[message.length() + 1];
	strcpy(messageBuffer, message.c_str());

	// echo received message back to client
	int sentBytes = connectedClient->send_all(messageBuffer, message.size());

	// close client if sending failed
	if (sentBytes == -1) {
		printf("  sending socket message '%s' failed\n", message.c_str());
		return false;
	}

	return true;
}

void SocketServer::addListener(SocketServer::SocketServerListener *socketServerListener) {
	listeners.push_back(socketServerListener);
}
