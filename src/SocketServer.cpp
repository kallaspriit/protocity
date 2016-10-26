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

        char buffer[256];

        while (true) {
			// check whether the connection is still valid
			if (!client.is_connected()) {
				printf("> socket connection to %s has been closed\n", client.get_address());

				client.close();

				break;
			}

			// attempt to receive some data
            int receivedBytes = client.receive(buffer, sizeof(buffer));

			// just try again if nothing received
            if (receivedBytes <= 0) {
				continue;
			}

            // print received message to terminal
            buffer[receivedBytes] = '\0';
            printf("> received message:'%s'\n", buffer);

            // echo received message back to client
            int sentBytes = client.send_all(buffer, receivedBytes);

			// close client if sending failed
            if (sentBytes == -1) {
				printf("  sending socket message '%s' to %s failed", buffer, client.get_address());

				client.close();

				break;
			}
        }
    }

	return true;
}
