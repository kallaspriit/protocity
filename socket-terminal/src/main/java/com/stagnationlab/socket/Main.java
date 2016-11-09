package com.stagnationlab.socket;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class Main {

    private class InputReader implements Runnable {

        private volatile boolean isRunning = true;
        private BufferedReader socketIn;

        InputReader(BufferedReader socketIn) {
            this.socketIn = socketIn;
        }

        void terminate() {
            System.out.printf("# terminating input reader%n");

            isRunning = false;
        }

        @Override
        public void run() {
            while (isRunning) {
                try {
                    if (!socketIn.ready()) {
                        continue;
                    }

                    String socketInput = socketIn.readLine();

                    if (socketInput != null) {
                        System.out.printf("< %s%n> ", socketInput);
                    }
                } catch (IOException e) {
                    e.printStackTrace();

                    isRunning = false;
                }
            }

            System.out.printf("# stopped input reader thread%n");
        }
    }

    public static void main(String[] args) throws Exception {
        (new Main()).run();
    }

    private void run() throws Exception {
        String hostName = "10.220.20.11";
        int portNumber = 8080;
        boolean isRunning = true;

        System.out.printf("### connecting to %s:%d ###%n", hostName, portNumber);

        Socket socket = new Socket(hostName, portNumber);
        PrintWriter socketOut = new PrintWriter(socket.getOutputStream(), true);
        BufferedReader socketIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        BufferedReader consoleIn = new BufferedReader(new InputStreamReader(System.in));

        System.out.printf("# connected, type 'quit' to exit%n> ");

        InputReader inputReader = new InputReader(socketIn);
        Thread inputThread = new Thread(inputReader);
        inputThread.start();

        while (isRunning) {
            String userInput = consoleIn.readLine();

            if (userInput.equals("quit")) {
                System.out.printf("# quitting%n");

                isRunning = false;

                break;
            }

            // System.out.println("> " + userInput);

            socketOut.printf("%s\n", userInput);

            // System.out.println("<" + socketReader.readLine());
        }

        inputReader.terminate();
        socket.close();
        inputThread.join();

        System.out.printf("### socket client stopped ###%n%n");
    }

}
