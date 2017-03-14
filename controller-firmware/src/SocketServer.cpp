#include "SocketServer.hpp"

SocketServer::SocketServer() {
	commandBuffer = new char[COMMAND_BUFFER_SIZE];
	receiveBuffer = new char[RECEIVE_BUFFER_SIZE];
	sendBuffer = new char[SEND_BUFFER_SIZE];
}

bool SocketServer::start(EthernetInterface *ethernetInterface, int port) {
	log.info("starting socket server on port %d", port);

	tpcSocketServer.set_blocking(true);

    int bindResult = tpcSocketServer.bind(port);

	if (bindResult == 0) {
		log.info("binding to port %d was successful", port);
	} else {
		log.warn("binding to port %d failed", port);

		return false;
	}

    int listenResult = tpcSocketServer.listen();

	if (listenResult == 0) {
		log.info("listening on port %d was successful", port);
	} else {
		log.warn("listening on port %d failed", port);

		return false;
	}

	listenThread.start(callback(this, &SocketServer::runListenThread));

	return true;
}

void SocketServer::runListenThread() {
	log.info("starting socket server listen thread");

	while (true) {
        log.info("waiting for new connection...");

        TCPSocketConnection client;
        int acceptResult = tpcSocketServer.accept(client);

		if (acceptResult != 0) {
			log.warn("accepting new client failed");

			break;
		}

		// we have our own thread, use blocking calls
        client.set_blocking(true);

        log.info("got socket connection from: %s", client.get_address());

		connectedClient = &client;

		// notify listeners
		for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
			(*it)->onSocketClientConnected(connectedClient);
		}

        while (connectedClient != NULL) {
			// check whether the connection is still valid
			if (!connectedClient->is_connected()) {
				log.info("socket connection to %s has been closed", connectedClient->get_address());

				dropConnection();

				continue;
			}

			// attempt to receive some data
            int receivedBytes = connectedClient->receive(receiveBuffer, RECEIVE_BUFFER_SIZE);

			// just try again if nothing received
            if (receivedBytes <= 0) {
				continue;
			}

			// extract commands from the received data
			handleReceivedData(receiveBuffer, receivedBytes);
        }
    }
}

void SocketServer::handleReceivedData(char *buffer, int receivedBytes) {
	// search for newline delimiting commands
	for (int i = 0; i < receivedBytes; i++) {
		char receivedChar = buffer[i];

		if (receivedChar == '\n') {
			// only display the message if no listeners have been registered
			if (listeners.size() == 0) {
				log.info("received command: '%s'", commandBuffer);
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

bool SocketServer::isClientConnected() {
	return connectedClient != NULL && connectedClient->is_connected();
}

TCPSocketConnection *SocketServer::getConnectedClient() {
	return connectedClient;
}

void SocketServer::dropConnection() {
	// notify listeners
	for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)->onSocketClientDisconnected(connectedClient);
	}

	if (connectedClient != NULL) {
		if (connectedClient->is_connected()) {
			connectedClient->close();
		}

		connectedClient = NULL;
	}

	log.info("dropped socket connection");
}

bool SocketServer::sendMessage(std::string message) {
	if (!isClientConnected()) {
		return false;
	}

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
		log.warn("sending socket message '%s' failed", message);

		dropConnection();

		return false;
	}

	return true;
}

void SocketServer::addListener(SocketServer::SocketServerListener *listener) {
	listeners.push_back(listener);
}
