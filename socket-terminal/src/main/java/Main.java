import java.util.Scanner;

import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.SocketClient;

@SuppressWarnings("SameParameterValue")
public class Main implements MessageTransport.MessageListener {

	// runtime configuration
	private String hostName = "127.0.0.1";
	private int portNumber = 8080;

	// program configuration
	private static final int CONNECT_TIMEOUT = 3000;
	private static final int PING_INTERVAL = 1000;
	private static final int RECONNECT_INTERVAL = 5000;

	private Scanner scanner;
	private SocketClient socketClient;
	private Thread inputThread;
	private boolean isRunning = false;
	private boolean isExpectingPingResponse = false;
	private boolean didUserQuit = false;
	private boolean reconnectOnceClosed = false;

	public static void main(String[] args) {
		try {
			(new Main()).run();
		} catch (Exception e) {
			System.out.printf("# error occured%n");

			e.printStackTrace();
		}
	}

	private void run() {
		scanner = new Scanner(System.in);
		scanner.useDelimiter("");

        hostName = askFor("Enter host", hostName);
        portNumber = Integer.parseInt(askFor("Enter port", Integer.toString(portNumber)));

        socketClient = new SocketClient(hostName, portNumber, RECONNECT_INTERVAL);
        socketClient.addMessageListener(this);

		System.out.printf("# connecting to %s:%d (timeout: %dms)%n", hostName, portNumber, CONNECT_TIMEOUT);

		socketClient.connect(CONNECT_TIMEOUT);
    }

    private boolean sendMessage(String format, Object... args) {
    	String message = String.format(format, args);

	    System.out.printf("> %s%n", message);

	    return socketClient.sendMessage("%s\n", message);
    }

	private boolean sendSilentMessage(String format, Object... args) {
		String message = String.format(format, args);

		return socketClient.sendMessage("%s\n", message);
	}

	private void shutdown() {
		if (!isRunning) {
			return;
		}

		isRunning = false;
		isExpectingPingResponse = false;
	}

    private String askFor(String question, String defaultValue) {
        System.out.printf("> %s (%s): ", question, defaultValue);

	    String userInput = scanner.nextLine();

	    if (userInput == null || userInput.length() == 0) {
            return defaultValue;
        }

        return userInput;
    }

	@Override
	public void onSocketOpen() {
		System.out.printf("# socket connection opened%n");

		isRunning = true;
		reconnectOnceClosed = false;

		Thread pingThread = new Thread(() -> {
			while (isRunning) {
				try {
					Thread.sleep(PING_INTERVAL);
				} catch (InterruptedException e) {
					System.out.printf("# sleeping ping thread failed%n");

					e.printStackTrace();
				}

				if (!isRunning || didUserQuit) {
					break;
				}

				if (isExpectingPingResponse) {
					System.out.printf("# ping response was not received, connection must have died%n");

					reconnectOnceClosed = true;

					shutdown();

					socketClient.close();

					break;
				}

				if (!sendSilentMessage("0:ping")) {
					System.out.printf("# sending ping failed, shutting down%n");

					shutdown();

					break;
				}

				isExpectingPingResponse = true;
			}
		});

		pingThread.start();

		if (inputThread == null) {
			inputThread = new Thread(() -> {
				while (!didUserQuit) {
					String userInput = scanner.nextLine().trim();

					if (userInput.length() == 0) {
						continue;
					}

					if (userInput.equals("quit")) {
						System.out.printf("# quitting%n");

						didUserQuit = true;

						shutdown();

						socketClient.close();

						continue;
					}

					if (!sendMessage(userInput)) {
						System.out.printf("# sending user message failed, shutting down%n");

						shutdown();
					}
				}
			});

			inputThread.start();
		}
	}

	@Override
	public void onSocketClose() {
		if (reconnectOnceClosed) {
			System.out.printf("# reconnecting%n");

			socketClient.reconnect();
		} else {
			System.out.printf("# socket connection closed%n");
		}
	}

	@Override
	public void onSocketMessageReceived(String message) {
		// don't display heartbeat messages
		if (message.length() >= 13 && message.substring(0, 11).equals("0:HEARTBEAT")) {
			return;
		}

		// ignore ping response
		if (message.equals("0:OK:pong")) {
			isExpectingPingResponse = false;

			return;
		}

		System.out.printf("< %s%n", message);
	}

	@Override
	public void onSocketConnectionFailed(Exception e) {
		if (reconnectOnceClosed) {
			System.out.printf("# reconnecting failed (%s - %s)%n", e.getClass().getSimpleName(), e.getMessage());
		} else {
			System.out.printf("# connecting failed (%s - %s)%n", e.getClass().getSimpleName(), e.getMessage());
		}
	}
}
