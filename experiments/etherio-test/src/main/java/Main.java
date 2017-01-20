import com.stagnationlab.etherio.*;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.List;

public class Main {

    private Commander commander;
    private PortController digitalOutPort;
    private PortController digitalInPort;
    private PortController analogOutPort;
    private PortController analogInPort;
    private PortController interruptPort;
    private PortController motionPort;
    private PortController luminosityPort;

    public static void main(String[] args) throws Exception {
        (new Main()).run();
    }

    private void run() throws Exception {
        String hostName = "127.0.0.1";
        int portNumber = 8080;

        BufferedReader consoleIn = new BufferedReader(new InputStreamReader(System.in));

        hostName = askFor("Enter host", hostName, consoleIn);
        portNumber = Integer.parseInt(askFor("Enter port", Integer.toString(portNumber), consoleIn));

        System.out.printf("# connecting to %s:%d.. ", hostName, portNumber);

        SocketClient socketClient = new SocketClient(hostName, portNumber);
        socketClient.connect();

        System.out.printf("success!%n");

        commander = new Commander(socketClient);

        // run various tests
        testCustomCommand();
        /*testDigitalOut();
        testDigitalIn();
        testAnalogOut();
        testAnalogIn();
        testInterrupt();
        testMotion();*/
	    testLuminosity();

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
        digitalOutPort = new PortController(1, commander);

        digitalOutPort.setPortMode(PortController.PortMode.DIGITAL_OUT);
        digitalOutPort.setDigitalValue(PortController.DigitalValue.HIGH);

        /*
        portController.sendCommand("port", 6, "listen", 0.05, 500);
        */
    }

    private void testDigitalIn() throws Exception {
        digitalInPort = new PortController(4, commander);

        digitalInPort.setPortMode(PortController.PortMode.DIGITAL_IN);
        digitalInPort.getDigitalValue().thenAccept(
                commandResponse -> System.out.printf("# port %d digital value: %s%n", digitalInPort.getId(), PortController.DigitalValue.valueOf(commandResponse.response.getString(0)))
        );
    }

    private void testAnalogOut() throws Exception {
        analogOutPort = new PortController(2, commander);

        analogOutPort.setPortMode(PortController.PortMode.ANALOG_OUT);
        analogOutPort.setAnalogValue(0.5f);
    }

    private void testAnalogIn() throws Exception {
        analogInPort = new PortController(6, commander);

        analogInPort.setPortMode(PortController.PortMode.ANALOG_IN);
        analogInPort.getAnalogValue().thenAccept(
                commandResponse -> System.out.printf("# port %d analog value: %f%n", analogInPort.getId(), commandResponse.response.getFloat(0))
        );

        analogInPort.listenAnalogValueChange(0.05f, 100, new PortController.PortEventListener() {

            @Override
            public void onPortAnalogValueChange(int id, float value) {
                System.out.printf("# port %d analog value changed to %f%n", id, value);

                analogOutPort.setAnalogValue(value);
            }

        });
    }

    private void testInterrupt() throws Exception {
        interruptPort = new PortController(4, commander);

        interruptPort.setPortMode(PortController.PortMode.INTERRUPT, new PortController.PortEventListener() {

            @Override
            public void onPortDigitalValueChange(int id, PortController.DigitalValue digitalValue) {
                System.out.printf("# port %d digital value changed to %s%n", id, digitalValue);

                digitalOutPort.setDigitalValue(digitalValue);
            }

            @Override
            public void onPortValueRise(int id) {
                System.out.printf("# port %d digital value became high%n", id);
            }

            @Override
            public void onPortValueFall(int id) {
                System.out.printf("# port %d digital value became low%n", id);
            }
        });
    }

    private void testMotion() throws Exception {
        motionPort = new PortController(5, commander);

        motionPort.setPortMode(PortController.PortMode.INTERRUPT, new PortController.PortEventListener() {

            @Override
            public void onPortDigitalValueChange(int id, PortController.DigitalValue digitalValue) {
                System.out.printf("# motion port %d digital value changed to %s%n", id, digitalValue);

                digitalOutPort.setDigitalValue(digitalValue);
            }
        });

        motionPort.setPullMode(PortController.PullMode.UP);
    }

	private void testLuminosity() throws Exception {
		luminosityPort = new PortController(1, commander);

		luminosityPort.sendPortCommand("TSL2561", "enable", 1000);
		luminosityPort.addEventListener(new PortController.PortEventListener() {
			@Override
			public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
				System.out.printf("# port %d capability '%s' measurement: %s %s%n", id, capabilityName, arguments.get(0), arguments.get(1));
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
