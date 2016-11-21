import com.stagnationlab.etherio.*;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Main implements PortController.PortEventListener {

    private PortController portController;

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

        portController = new PortController(socketClient);

        portController.addEventListener(this);

        test();

        socketClient.close();
    }

    private void test() throws Exception {
        // available memory
        portController.sendCommand("memory").thenAccept(
                commandResponse -> System.out.printf("# got memory request response: %d bytes%n", commandResponse.response.getInt(0))
        );

        // digital out
        portController.sendCommand("port", 1, "mode", "OUTPUT");
        portController.sendCommand("port", 1, "value", "HIGH");

        // pwm out
        portController.sendCommand("port", 2, "mode", "PWM");
        portController.sendCommand("port", 2, "value", 0.25);

        // interrupt
        portController.sendCommand("port", 4, "mode", "INTERRUPT");

        // analog in
        portController.sendCommand("port", 6, "mode", "ANALOG");
        portController.sendCommand("port", 6, "read").thenAccept(
                commandResponse -> System.out.printf("# port %d analog value: %f%n", commandResponse.command.getInt(0), commandResponse.response.getFloat(0))
        );
        portController.sendCommand("port", 6, "listen", 0.05, 500);

        // give some time to respond
        Thread.sleep(10000);
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
