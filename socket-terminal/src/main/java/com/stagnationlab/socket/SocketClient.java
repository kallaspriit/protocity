package com.stagnationlab.socket;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.*;

public class SocketClient {

    private class InputReader implements Runnable {

        private volatile boolean isRunning = true;
        private BufferedReader socketIn;

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

    public interface MessageListener {
        void onSocketMessageReceived(String message);
    }

    private String hostName = "127.0.0.1";
    private int portNumber = 8080;

    private Socket socket;
    private PrintWriter socketOut;
    private BufferedReader socketIn;
    private InputReader inputReader;
    private Thread inputThread;

    private boolean isConnected = false;
    private Queue<String> inputQueue;
    private List<MessageListener> messageListeners;

    public SocketClient() {
        inputQueue = new LinkedList<>();
        messageListeners = new ArrayList<>();
    }

    public SocketClient(String hostName, int portNumber) {
        this();

        setRemoteHost(hostName, portNumber);
    }

    public void setRemoteHost(String hostName, int portNumber) {
        this.hostName = hostName;
        this.portNumber = portNumber;
    }

    public void connect() throws IOException {
        socket = new Socket(hostName, portNumber);
        socketOut = new PrintWriter(socket.getOutputStream(), true);
        socketIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));

        inputReader = new InputReader(socketIn);
        inputThread = new Thread(inputReader);
        inputThread.start();

        isConnected = true;
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

    public String getQueuedMessage() {
        String message = inputQueue.peek();

        if (message == null) {
            return null;
        }

        inputQueue.remove();

        return message;
    }

    public void close() throws IOException {
        inputReader.close();
        socket.close();
    }
}