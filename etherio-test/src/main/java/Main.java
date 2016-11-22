import com.stagnationlab.etherio.*;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Main {

    private Commander commander;

    public static void main(String[] args) throws Exception {
        (new Main()).run();
    }

    private void run() throws Exception {
        // String hostName = "127.0.0.1";
        String hostName = "10.220.20.17";
        int portNumber = 8080;

        BufferedReader consoleIn = new BufferedReader(new InputStreamReader(System.in));

        hostName = askFor("Enter host", hostName, consoleIn);
        portNumber = Integer.parseInt(askFor("Enter port", Integer.toString(portNumber), consoleIn));

        System.out.printf("# connecting to %s:%d.. ", hostName, portNumber);

        SocketClient socketClient = new SocketClient(hostName, portNumber);
        socketClient.connect();

        System.out.printf("success!%n");

        commander = new Commander(socketClient);

        testCustomCommand();
        testDigitalOut();
        testDigitalIn();
        testPwm();
        testInterrupt();
        testAnalog();

        // give some time to respond
        Thread.sleep(10000);

        socketClient.close();
    }

    private void testCustomCommand() throws Exception {
        commander.sendCommand("memory").thenAccept(
                commandResponse -> System.out.printf("# got memory request response: %d bytes%n", commandResponse.response.getInt(0))
        );
    }

    private void testDigitalOut() throws Exception {
        PortController portController = new PortController(1, commander);

        portController.setPortMode(PortController.PortMode.DIGITAL_OUT);
        portController.setDigitalValue(PortController.DigitalValue.HIGH);

        /*
        portController.sendCommand("port", 6, "listen", 0.05, 500);
        */
    }

    private void testDigitalIn() throws Exception {
        PortController portController = new PortController(4, commander);

        portController.setPortMode(PortController.PortMode.DIGITAL_IN);
        portController.getDigitalValue().thenAccept(
                commandResponse -> System.out.printf("# port %d digital value: %s%n", portController.getId(), PortController.DigitalValue.valueOf(commandResponse.response.getString(0)))
        );
    }

    private void testPwm() throws Exception {
        PortController portController = new PortController(2, commander);

        portController.setPortMode(PortController.PortMode.PWM);
        portController.setAnalogValue(0.5f);
    }

    private void testInterrupt() throws Exception {
        PortController portController = new PortController(4, commander);

        portController.setPortMode(PortController.PortMode.INTERRUPT);
        portController.addEventListener(new PortController.PortEventListener() {

            @Override
            public void onPortDigitalValueChange(int id, PortController.DigitalValue digitalValue) {
                System.out.printf("# port %d digital value changed to %s%n", id, digitalValue);
            }

        });
    }

    private void testAnalog() throws Exception {
        PortController portController = new PortController(6, commander);

        portController.setPortMode(PortController.PortMode.ANALOG);

        portController.addEventListener(new PortController.PortEventListener() {

            @Override
            public void onPortAnalogValueChange(int id, float value) {
                System.out.printf("# port %d analog value changed to %f%n", id, value);
            }

        });
    }

    private static String askFor(String question, String defaultValue, BufferedReader consoleIn) throws IOException {
        System.out.printf("@ %s (%s): ", question, defaultValue);

        String userInput = consoleIn.readLine();

        if (userInput.length() == 0) {
            return defaultValue;
        }

        return userInput;
    }
}
