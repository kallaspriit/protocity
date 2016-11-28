#include "SocketServer.hpp"

bool SocketServer::start(EthernetInterface *ethernetInterface, int port) {
	printf("# starting socket server on port %d\n", port);

    int bindResult = tpcSocketServer.bind(port);

	if (bindResult == 0) {
		printf("#  binding to port %d was successful\n", port);
	} else {
		printf("#  binding to port %d failed\n", port);

		return false;
	}

    int listenResult = tpcSocketServer.listen();

	if (listenResult == 0) {
		printf("#  listening on port %d was successful\n", port);
	} else {
		printf("#  listening on port %d failed\n", port);

		return false;
	}

	listenThread.start(this, &SocketServer::runListenThread);

	return true;
}

void SocketServer::runListenThread() {
	printf("# starting socket server listen thread\n");

	while (true) {
        printf("# waiting for new connection...\n");

        TCPSocketConnection client;
        int acceptResult = tpcSocketServer.accept(client);

		if (acceptResult != 0) {
			printf("# accepting new client failed\n");

			break;
		}

        client.set_blocking(false, SOCKET_RECEIVE_TIMEOUT_MS);

        printf("# got socket connection from: %s\n", client.get_address());

		connectedClient = &client;
		isConnected = true;

		// notify listeners
		for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
			(*it)->onSocketClientConnected(connectedClient);
		}

        char buffer[256];

        while (isConnected) {
			// check whether the connection is still valid
			if (!client.is_connected()) {
				printf("# socket connection to %s has been closed\n", client.get_address());

				dropConnection();

				continue;
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
						printf("# received command: '%s'\n", commandBuffer);
					}

					// notify all message listeners
					for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
						(*it)->onSocketCommandReceived(commandBuffer, commandLength);
					}

					// send response
					// sendMessage("got command '" + messageBuffer + "'");

					commandBuffer[0] = '\0';
					commandLength = 0;
				} else {
					if (commandLength > MAX_COMMAND_LENGTH - 1) {
						return;
					}

					commandBuffer[commandLength++] = receivedChar;
					commandBuffer[commandLength] = '\0';
				}
			}
        }
    }
}

bool SocketServer::isClientConnected() {
	return isConnected && connectedClient != NULL && connectedClient->is_connected();
}

TCPSocketConnection *SocketServer::getConnectedClient() {
	return connectedClient;
}

void SocketServer::dropConnection() {
	printf("# dropping socket connection\n");

	// notify listeners
	for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)->onSocketClientDisconnected(connectedClient);
	}

	if (connectedClient != NULL) {
		connectedClient->close();
		connectedClient = NULL;
	}

	isConnected = false;
}

bool SocketServer::sendMessage(std::string message) {
	strcpy(sendBuffer, message.c_str());

	bool result = sendMessage(sendBuffer, message.length());

	return result;
}

bool SocketServer::sendMessage(char *message, int length) {
	if (!isClientConnected()) {
		return false;
	}

	// echo received message back to client
	int sentBytes = connectedClient->send_all(message, length);

	// delete [] message;

	// close client if sending failed
	if (sentBytes == -1) {
		printf("# sending socket message '%s' failed\n", message);

		dropConnection();

		return false;
	}

	return true;
}

void SocketServer::addListener(SocketServer::SocketServerListener *listener) {
	listeners.push_back(listener);
}
