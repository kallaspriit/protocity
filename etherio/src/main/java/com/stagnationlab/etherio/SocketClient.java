package com.stagnationlab.etherio;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.*;

import lombok.extern.slf4j.Slf4j;

@SuppressWarnings({"WeakerAccess", "unused"})
@Slf4j
public class SocketClient implements MessageTransport {

	interface PingSender {

		void ping(SocketClient socketClient);

	}

	// configuration
    private String hostName = "127.0.0.1";
    private int portNumber = 8080;
	private int reconnectTimeout = 5000;
	private int pingInterval = 1000;

	// dependencies
    private Socket socket;
	private BufferedReader socketIn;
    private PrintWriter socketOut;
    private Thread inputThread;

    // runtime info
    private boolean isRunning = false;
    private final Queue<String> inputQueue = new LinkedList<>();
	private final Queue<Integer> latencies = new LinkedList<>();
    private final List<MessageListener> messageListeners = new ArrayList<>();
    private int messageCounter = 1;
    private boolean wasEverConnected = false;
    private PingSender pingSender = null;
    private int lastConnectionTimeout = 0;

	public SocketClient(String hostName, int portNumber, int reconnectTimeout) {
		setRemoteHost(hostName, portNumber);
		setReconnectTimeout(reconnectTimeout);
	}

    private void setRemoteHost(String hostName, int portNumber) {
        this.hostName = hostName;
        this.portNumber = portNumber;
    }

    @SuppressWarnings("SameParameterValue")
    public void connect(int connectionTimeout) {
	    log.debug("connecting to {}:{}", hostName, portNumber);

	    lastConnectionTimeout = connectionTimeout;

	    try {
		    socket = new Socket();
		    socket.connect(new InetSocketAddress(hostName, portNumber), connectionTimeout);
		    socketOut = new PrintWriter(socket.getOutputStream(), true);
		    socketIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));

		    inputThread = new Thread(getInputThreadTask());

		    inputThread.start();

		    isRunning = true;
		    wasEverConnected = true;

		    log.debug("connected to {}:{}", hostName, portNumber);

		    for (MessageListener messageListener : messageListeners) {
			    messageListener.onSocketOpen();
		    }
	    } catch (IOException e) {
		    for (MessageListener messageListener : messageListeners) {
			    messageListener.onSocketConnectionFailed(e);
		    }

	    	if (wasEverConnected && reconnectTimeout >= 0) {
			    log.debug("connecting to {}:{} failed, reconnecting in {}ms", hostName, portNumber, reconnectTimeout);

			    scheduleReconnect(reconnectTimeout);
		    } else {
			    log.warn("connecting to {}:{} failed, reconnecting is not enabled", hostName, portNumber);
		    }
	    }
    }

    public Runnable getInputThreadTask() {
		return () -> {
			boolean wasConnectionLost = false;

			while (isRunning) {
				try {
					String message = socketIn.readLine();

					if (message != null) {
						log.trace("received message: '{}'", message);

						inputQueue.add(message);

						synchronized (this) {
							for (MessageListener messageListener : messageListeners) {
								messageListener.onSocketMessageReceived(message);
							}
						}
					}
				} catch (Exception e) {
					log.warn("reading from {}:{} failed, stopping input loop ({} - {})", hostName, portNumber, e.getClass().getSimpleName(), e.getMessage());

					// if the socket client was still running during the error, the connection was lost
					if (isRunning) {
						wasConnectionLost = true;
					}

					break;
				}
			}

			if (wasConnectionLost) {
				log.debug("connection to {}:{} was lost, closing socket", hostName, portNumber);

				close();

				if (wasEverConnected && reconnectTimeout >= 0) {
					log.debug("connection to {}:{} was lost, reconnecting in {}ms", hostName, portNumber, reconnectTimeout);

					scheduleReconnect(reconnectTimeout);
				} else {
					log.debug("connection to {}:{} was lost, reconnecting is not enabled", hostName, portNumber);
				}
			} else {
				log.debug("input thread completed");
			}
		};
    }

    public void addMessageListener(MessageListener messageListener) {
        messageListeners.add(messageListener);
    }

    public boolean sendMessage(String format, Object...arguments) {
    	if (socketOut == null) {
		    log.warn("requested sending message '{}' but the output stream is not available", format);

    		return false;
	    }

	    log.trace("sending message: '{}'", format);

        socketOut.format(format, arguments);

        return !socketOut.checkError();
    }

    public int getMessageCount() {
        return inputQueue.size();
    }

    public String getMessage() {
        String message = inputQueue.peek();

        if (message == null) {
            return null;
        }

        inputQueue.remove();

        return message;
    }

    public int getNextMessageId() {
        return messageCounter++;
    }

    public void close() {
    	if (!isRunning) {
    		return;
	    }

	    log.debug("closing socket connection to {}:{}", hostName, portNumber);

	    isRunning = false;

	    try {
	    	log.debug("waiting for input thread to join..");
		    inputThread.join(1000);
		    log.debug("input thread has successfully joined");
	    } catch (InterruptedException e) {
	    	log.warn("joining input thread failed ({} - {})", e.getClass().getSimpleName(), e.getMessage());

		    e.printStackTrace();
	    }

	    try {
		    socket.close();
	    } catch (IOException e) {
		    log.warn("closing socket failed ({} - {})", e.getClass().getSimpleName(), e.getMessage());

		    e.printStackTrace();
	    }

	    for (MessageListener messageListener : messageListeners) {
		    messageListener.onSocketClose();
	    }

	    log.debug("socket to {}:{} has been closed", hostName, portNumber);
    }

	public void scheduleReconnect(int timeout) {
		if (timeout == 0) {
			reconnect();
		} else if (timeout > 0) {
			log.debug("scheduling reconnect in {}ms", timeout);

			setTimeout(this::reconnect, timeout);
		}
	}

	public void reconnect() {
		if (isRunning) {
			log.debug("reconnect requested but the connection is already open, closing existing");

			close();
		}

		log.debug("attempting to reconnect");

		connect(lastConnectionTimeout);
	}

	public void setReconnectTimeout(int reconnectTimeout) {
		this.reconnectTimeout = reconnectTimeout;
	}

	private void setTimeout(Runnable runnable, int delay){
		new Thread(() -> {
			try {
				Thread.sleep(delay);

				runnable.run();
			}  catch (Exception e) {
				log.warn("timeout error occured ({} - {})", e.getClass().getSimpleName(), e.getMessage());

				e.printStackTrace();
			}
		}).start();
	}
}