package com.stagnationlab.etherio;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.*;

public class SocketClient implements MessageTransport {

    private class InputReader implements Runnable {

        private volatile boolean isRunning = true;
        private final BufferedReader socketIn;

        InputReader(BufferedReader socketIn) {
            this.socketIn = socketIn;
        }

        void close() {
            isRunning = false;
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
                    }

                    for (MessageListener messageListener : messageListeners) {
                        messageListener.onSocketMessageReceived(message);
                    }
                } catch (IOException e) {
                    e.printStackTrace();

                    isRunning = false;
                }
            }
        }
    }

    private String hostName = "127.0.0.1";
    private int portNumber = 8080;

    private Socket socket;
    private PrintWriter socketOut;
    private InputReader inputReader;

    private final Queue<String> inputQueue;
    private final List<MessageListener> messageListeners;
    private int messageCounter = 0;

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

    public void connect() throws IOException {
        socket = new Socket(hostName, portNumber);
        socketOut = new PrintWriter(socket.getOutputStream(), true);
        BufferedReader socketIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));

        inputReader = new InputReader(socketIn);
        Thread inputThread = new Thread(inputReader);
        inputThread.start();

        boolean isConnected = true;
    }

    public void addMessageListener(MessageListener messageListener) {
        messageListeners.add(messageListener);
    }

    public void sendMessage(String format, Object...arguments) {
        socketOut.format(format, arguments);
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

    public void close() throws IOException {
        inputReader.close();
        socket.close();
    }
}