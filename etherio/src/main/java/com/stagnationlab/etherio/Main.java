package com.stagnationlab.etherio;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Main implements PortController.PortEventListener {

    public static void main(String[] args) throws Exception {
        (new Main()).run();
    }

    private void run() throws Exception {
        // String hostName = "127.0.0.1";
        String hostName = "10.220.20.11";
        int portNumber = 8080;

        BufferedReader consoleIn = new BufferedReader(new InputStreamReader(System.in));

        hostName = askFor("Enter host", hostName, consoleIn);
        portNumber = Integer.parseInt(askFor("Enter port", Integer.toString(portNumber), consoleIn));

        System.out.printf("# connecting to %s:%d.. ", hostName, portNumber);

        SocketClient socketClient = new SocketClient(hostName, portNumber);
        socketClient.connect();

        System.out.printf("success!%n");

        PortController portController = new PortController(socketClient);

        portController.addEventListener(this);

        // test communication
        portController.test();

        /*
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
        */

        // give some time to respond
        Thread.sleep(10000);

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

    @Override
    public void onPortDigitalValueChange(int id, PortController.DigitalValue value) {
        System.out.printf("# port %d digital value changed to %s%n", id, value.name());
    }

    @Override
    public void onPortAnalogValueChange(int id, float value) {
        System.out.printf("# port %d analog value changed to %f%n", id, value);
    }

    @Override
    public void onPortValueRise(int id) {
        System.out.printf("# port %d interrupt rose%n", id);
    }

    @Override
    public void onPortValueFall(int id) {
        System.out.printf("# port %d interrupt fell%n", id);
    }
}
