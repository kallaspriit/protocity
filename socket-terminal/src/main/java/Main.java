import java.io.IOException;
import java.util.Scanner;

import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.SocketClient;

@SuppressWarnings("SameParameterValue")
public class Main {

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
	private boolean wasEverConnected = false;
	private boolean didUserQuit = false;

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

        socketClient = new SocketClient(hostName, portNumber);

        socketClient.addMessageListener(new MessageTransport.MessageListener() {
	        @Override
	        public void onSocketOpen() {
		        handleSocketOpen();
	        }

	        @Override
	        public void onSocketClose() {
	        	handleSocketClose();
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
        });

	    connect();
    }

	private void connect() {
	    System.out.printf("# connecting to %s:%d (timeout: %dms)%n", hostName, portNumber, CONNECT_TIMEOUT);

	    try {
		    socketClient.connect(CONNECT_TIMEOUT);
	    } catch (IOException e) {
		    System.out.printf("# connection failed (%s - %s)%n", e.getClass().getSimpleName(), e.getMessage());

		    shutdown();

		    if (wasEverConnected) {
			    scheduleReconnectAttempt();
		    }
	    }
    }

    private void handleSocketOpen() {
	    System.out.printf("# socket connection opened%n");

	    isRunning = true;
	    wasEverConnected = true;

	    Thread pingThread = new Thread(() -> {
		    while (isRunning) {
			    try {
				    Thread.sleep(PING_INTERVAL);
			    } catch (InterruptedException e) {
				    System.out.printf("# sleeping ping thread failed%n");

				    e.printStackTrace();
			    }

			    if (isExpectingPingResponse) {
				    System.out.printf("# ping response was not received, connection must have died%n");

				    shutdown();

				    continue;
			    }

			    if (!sendSilentMessage("0:ping")) {
				    System.out.printf("# sending ping failed, shutting down%n");

				    shutdown();

				    continue;
			    }

			    isExpectingPingResponse = true;
		    }
	    });

	    pingThread.start();

	    if (inputThread == null) {
		    inputThread = new Thread(() -> {
			    while (!didUserQuit) {
				    /*
				    // don't block the thread if no input is available
				    try {
					    if (System.in.available() == 0) {
						    Thread.sleep(100);

						    continue;
					    }
				    } catch (Exception e) {
					    e.printStackTrace();
				    }
				    */

				    String userInput = scanner.nextLine().trim();

				    if (userInput.length() == 0) {
					    continue;
				    }

				    if (userInput.equals("quit")) {
					    System.out.printf("# quitting%n");

					    didUserQuit = true;

					    shutdown();

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

	    try {
		    pingThread.join();
		    //inputThread.join();
	    } catch (InterruptedException e) {
		    System.out.printf("# joining threads failed%n");

		    e.printStackTrace();
	    }

	    socketClient.close();
    }

	private void handleSocketClose() {
		System.out.printf("# socket connection closed%n");

		if (isRunning) {
			shutdown();
		}

		if (wasEverConnected && !didUserQuit) {
			scheduleReconnectAttempt();
		}
	}

	private void scheduleReconnectAttempt() {
		System.out.printf("# trying to reconnect in %dms%n", RECONNECT_INTERVAL);

		try {
			Thread.sleep(RECONNECT_INTERVAL);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		connect();
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

}
