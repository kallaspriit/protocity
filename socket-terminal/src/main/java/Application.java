import java.util.Scanner;

import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.SocketClient;

@SuppressWarnings("SameParameterValue")
public class Application implements MessageTransport.MessageListener, SocketClient.PingSender {

	// runtime configuration
	private String hostName = "127.0.0.1";
	private int portNumber = 8080;

	// program configuration
	private static final int CONNECT_TIMEOUT = 3000;
	private static final int RECONNECT_TIMEOUT = 5000;
	private static final int PING_INTERVAL = 5000;

	private Scanner scanner;
	private SocketClient socketClient;
	private Thread inputThread;
	private boolean didUserQuit = false;

	public static void main(String[] args) {
		(new Application()).start();
	}

	private void start() {
		scanner = new Scanner(System.in);
		scanner.useDelimiter("");

        hostName = askFor("Enter host", hostName);
        portNumber = Integer.parseInt(askFor("Enter port", Integer.toString(portNumber)));

        socketClient = new SocketClient(hostName, portNumber, RECONNECT_TIMEOUT);
        socketClient.addMessageListener(this);
        socketClient.setPingSender(this, PING_INTERVAL);
		socketClient.connect(CONNECT_TIMEOUT);
    }

    private boolean sendMessage(String format, Object... args) {
    	String message = String.format(format, args);

	    System.out.printf("> %s%n", message);

	    return socketClient.sendMessage("%s\n", message);
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
	public void onSocketConnecting(boolean isReconnecting) {
		if (isReconnecting) {
			System.out.printf("# reconnecting%n");
		} else {
			System.out.printf("# connecting to %s:%d (timeout: %dms)%n", hostName, portNumber, CONNECT_TIMEOUT);
		}
	}

	@Override
	public void onSocketOpen() {
		System.out.printf("# socket connection opened%n");

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

						socketClient.close();

						continue;
					}

					if (!socketClient.isConnected()) {
						System.out.printf("# sending message '%s' failed (not connected)%n", userInput);

						continue;
					}

					if (!sendMessage(userInput)) {
						System.out.printf("# sending message '%s' failed (operation failed, probably lost connection)%n", userInput);
					}
				}
			});

			inputThread.start();
		}
	}

	@Override
	public void onSocketClose() {
		System.out.printf("# socket connection closed%n");
	}

	@Override
	public void onSocketMessageReceived(String message) {
		// don't display heartbeat messages
		if (message.length() >= 13 && message.substring(0, 11).equals("0:HEARTBEAT")) {
			return;
		}

		System.out.printf("< %s%n", message);
	}

	@Override
	public void onSocketConnectionFailed(Exception e) {
		System.out.printf("# connecting failed (%s - %s)%n", e.getClass().getSimpleName(), e.getMessage());
	}

	@Override
	public String getPingMessage() {
		return "0:ping\n";
	}

	@Override
	public boolean isPingResponse(String response) {
		return response.equals("0:OK:pong");
	}
}
