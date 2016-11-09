package com.stagnationlab.socket;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Main {

    public static void main(String[] args) throws Exception {
        String hostName = "127.0.0.1";
        int portNumber = 8080;

        BufferedReader consoleIn = new BufferedReader(new InputStreamReader(System.in));

        hostName = askFor("Enter host", hostName, consoleIn);
        portNumber = Integer.parseInt(askFor("Enter port", Integer.toString(portNumber), consoleIn));

        System.out.printf("### connecting to %s:%d ###%n", hostName, portNumber);

        SocketClient socketClient = new SocketClient(hostName, portNumber);
        socketClient.connect();

        System.out.printf("# connected, type 'quit' to exit%n> ");

        socketClient.addMessageListener(message -> {
            System.out.printf("> %s%n> ", message);
        });

        while (true) {
            String userInput = consoleIn.readLine();

            if (userInput.equals("quit")) {
                System.out.printf("# quitting%n");

                break;
            }

            socketClient.sendMessage("%s\n", userInput);
        }

        socketClient.close();
    }

    private static String askFor(String question, String defaultValue, BufferedReader consoleIn) throws IOException {
        System.out.printf("> %s (%s): ", question, defaultValue);

        String userInput = consoleIn.readLine();

        if (userInput.length() == 0) {
            return defaultValue;
        }

        return userInput;
    }

}
