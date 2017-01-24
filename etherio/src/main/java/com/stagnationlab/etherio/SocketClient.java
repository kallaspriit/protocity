package com.stagnationlab.etherio;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.*;

@SuppressWarnings({"WeakerAccess", "unused"})
public class SocketClient implements MessageTransport, Runnable {

    private String hostName = "127.0.0.1";
    private int portNumber = 8080;

    private Socket socket;
	private BufferedReader socketIn;
    private PrintWriter socketOut;
    private Thread inputThread;

    private boolean isRunning = false;
    private final Queue<String> inputQueue;
    private final List<MessageListener> messageListeners;
    private int messageCounter = 1;

    private SocketClient() {
        inputQueue = new LinkedList<>();
        messageListeners = new ArrayList<>();
    }

    public SocketClient(String hostName, int portNumber) {
        this();

        setRemoteHost(hostName, portNumber);
    }

    private void setRemoteHost(String hostName, int portNumber) {
        this.hostName = hostName;
        this.portNumber = portNumber;
    }

    public void connect(int connectionTimeout) throws IOException {
        // socket = new Socket(hostName, portNumber);
        socket = new Socket();
	    socket.connect(new InetSocketAddress(hostName, portNumber), connectionTimeout);
	    isRunning = true;

	    socketOut = new PrintWriter(socket.getOutputStream(), true);
        socketIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));

        inputThread = new Thread(this);
        inputThread.start();

	    for (MessageListener messageListener : messageListeners) {
		    messageListener.onSocketOpen();
	    }
    }

	public void connect() throws IOException {
    	connect(5000);
	}

    public void addMessageListener(MessageListener messageListener) {
        messageListeners.add(messageListener);
    }

    public boolean sendMessage(String format, Object...arguments) {
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

	@Override
	public void run() {
		while (isRunning) {
			try {
				if (!socketIn.ready()) {
					continue;
				}

				String message = socketIn.readLine();

				if (message != null) {
					inputQueue.add(message);

					synchronized (this) {
						for (MessageListener messageListener : messageListeners) {
							messageListener.onSocketMessageReceived(message);
						}
					}
				}
			} catch (IOException e) {
				e.printStackTrace();

				close();
			}
		}
	}

    public void close() {
	    isRunning = false;

	    try {
		    inputThread.join();
	    } catch (InterruptedException e) {
		    e.printStackTrace();
	    }

	    try {
		    socket.close();
	    } catch (IOException e) {
		    e.printStackTrace();
	    }

	    for (MessageListener messageListener : messageListeners) {
		    messageListener.onSocketClose();
	    }
    }
}