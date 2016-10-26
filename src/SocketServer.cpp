#include "SocketServer.hpp"

#include "EthernetInterface.h"

bool SocketServer::start(EthernetInterface *ethernetInterface, int port) {
	TCPSocketServer server;

	printf("> starting socket server on port %d\n", port);

    int bindResult = server.bind(port);

	if (bindResult == 0) {
		printf("  binding to port %d was successful\n", port);
	} else {
		error("  binding to port %d failed\n", port);
	}

    int listenResult = server.listen();

	if (listenResult == 0) {
		printf("  listening on port %d was successful\n", port);
	} else {
		error("  listening on port %d failed\n", port);
	}

    while (true) {
        printf("> waiting for new connection...\n");

        TCPSocketConnection client;
        int acceptResult = server.accept(client);

		if (acceptResult == 0) {
			printf("  accepting new client\n");
		} else {
			printf("  accepting new client failed\n");

			break;
		}

        client.set_blocking(false, SOCKET_RECEIVE_TIMEOUT_MS);

        printf("> got socket connection from: %s\n", client.get_address());

		connectedClient = &client;

        char buffer[256];

        while (true) {
			// check whether the connection is still valid
			if (!client.is_connected()) {
				printf("> socket connection to %s has been closed\n", client.get_address());

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
					printf("> received command: '%s'\n", messageBuffer.c_str());

					for (std::vector<MessageListener*>::iterator it = messageListeners.begin(); it != messageListeners.end(); ++it) {
						(*it)->onSocketMessageReceived(messageBuffer);
					}

					// send response
					sendMessage("got command '" + messageBuffer + "'");

					messageBuffer = "";
				} else {
					messageBuffer += receivedChar;
				}
			}
        }
    }

	return true;
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

		connectedClient->close();
		connectedClient = NULL;

		return false;
	}

	return true;
}

void SocketServer::addMessageListener(SocketServer::MessageListener *messageListener) {
	messageListeners.push_back(messageListener);
}
