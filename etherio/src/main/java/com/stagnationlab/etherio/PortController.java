package com.stagnationlab.etherio;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class PortController implements MessageTransport.MessageListener {

    enum PortMode {
        UNUSED,
        INVALID,
        OUTPUT,
        INPUT,
        INTERRUPT,
        PWM,
        ANALOG
    };

    enum DigitalValue {
        LOW,
        HIGH
    };

    interface PortEventListener {

        void onPortDigitalValueChange(int id, DigitalValue value);
        void onPortAnalogValueChange(int id, float value);
        void onPortValueRise(int id);
        void onPortValueFall(int id);
    };

    private MessageTransport messageTransport;
    private List<PortEventListener> portEventListeners;
    private Map<Integer, Command> commandIdToCommandMap;

    public PortController(MessageTransport messageTransport) {
        this.messageTransport = messageTransport;
        this.portEventListeners = new ArrayList<>();
        this.commandIdToCommandMap = new HashMap<>();

        messageTransport.addMessageListener(this);
    }

    public void addEventListener(PortEventListener listener) {
        portEventListeners.add(listener);
    }

    public void test() {
        // available memory
        sendCommand("memory");

        // digital out
        sendCommand("port", 1, "mode", "OUTPUT");
        sendCommand("port", 1, "value", "HIGH");

        // pwm out
        sendCommand("port", 2, "mode", "PWM");
        sendCommand("port", 2, "value", 0.25);

        // interrupt
        sendCommand("port", 4, "mode", "INTERRUPT");

        // analog in
        sendCommand("port", 6, "mode", "ANALOG");
        sendCommand("port", 6, "read");
    }

    void sendCommand(String name, Object... arguments) {
        Command command = new Command(messageTransport.getNextMessageId(), name, arguments);

        sendCommand(command);
    }

    void sendCommand(Command command) {
        commandIdToCommandMap.put(command.id, command);

        String message = command.toString();

        System.out.printf("< %s%n", message);

        messageTransport.sendMessage("%s\n", message);
    }

    @Override
    public void onSocketMessageReceived(String message) {
        try {
            Command responseCommand = Command.parse(message);

            System.out.printf("> %s%n", message);

            handleResponse(responseCommand);
        } catch (Exception e) {
            System.out.printf("# got invalid response '%s'%n", message);
        }
    }

    private void handleResponse(Command responseCommand) {
        Command originalCommand = getCommandById(responseCommand.id);

        if (originalCommand == null) {
            System.out.printf("# original command for %s was not found%n", responseCommand.toString());

            return;
        }

        System.out.printf("# got response %s for original command %s%n", responseCommand.toString(), originalCommand.toString());
    }

    private Command getCommandById(int id) {
        if (!commandIdToCommandMap.containsKey(id)) {
            return null;
        }

        return commandIdToCommandMap.get(id);
    }
}
