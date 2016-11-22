import com.stagnationlab.etherio.*;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Main implements PortController.PortEventListener {

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
        testPwm();

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

        // digital out
        portController.setPortMode(PortController.PortMode.OUTPUT);
        portController.setValue(PortController.DigitalValue.HIGH);

        /*
        // available memory
        portController.sendCommand("memory").thenAccept(
                commandResponse -> System.out.printf("# got memory request response: %d bytes%n", commandResponse.response.getInt(0))
        );


        // portController.sendCommand("port", 1, "mode", "OUTPUT");
        // portController.sendCommand("port", 1, "value", "HIGH");
        portController.setPortMode(PortController.PortMode.OUTPUT);
        portController.setValue(PortController.DigitalValue.HIGH);

        // interrupt
        portController.sendCommand("port", 4, "mode", "INTERRUPT");

        // analog in
        portController.sendCommand("port", 6, "mode", "ANALOG");
        portController.sendCommand("port", 6, "read").thenAccept(
                commandResponse -> System.out.printf("# port %d analog value: %f%n", commandResponse.command.getInt(0), commandResponse.response.getFloat(0))
        );
        portController.sendCommand("port", 6, "listen", 0.05, 500);
        */
    }

    private void testPwm() throws Exception {
        PortController portController = new PortController(2, commander);

        // pwm out
        portController.setPortMode(PortController.PortMode.PWM);
        portController.setPwmDutyCycle(0.5f);
    }

    private static String askFor(String question, String defaultValue, BufferedReader consoleIn) throws IOException {
        System.out.printf("@ %s (%s): ", question, defaultValue);

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
