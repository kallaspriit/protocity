package com.stagnationlab.etherio;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;

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

    class CommandPromise {
        public Command command;
        CompletableFuture<Command> promise;

        public CommandPromise(Command command, CompletableFuture<Command> promise) {
            this.command = command;
            this.promise = promise;
        }
    }

    private MessageTransport messageTransport;
    private List<PortEventListener> portEventListeners;
    private Map<Integer, CommandPromise> commandPromises;

    public PortController(MessageTransport messageTransport) {
        this.messageTransport = messageTransport;
        this.portEventListeners = new ArrayList<>();
        this.commandPromises = new HashMap<>();

        messageTransport.addMessageListener(this);
    }

    public void addEventListener(PortEventListener listener) {
        portEventListeners.add(listener);
    }

    public void test() {
        // available memory
        sendCommand("memory").thenAccept(command -> {
            System.out.printf("# got memory request response: %d bytes%n", command.getInt(0));
        });

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
        sendCommand("port", 6, "read").thenAccept(command -> {
            System.out.printf("# port 6 analog value: %f%n", command.getFloat(0));
        });
    }

    CompletableFuture<Command> sendCommand(String name, Object... arguments) {
        Command command = new Command(messageTransport.getNextMessageId(), name, arguments);

        return sendCommand(command);
    }

    CompletableFuture<Command> sendCommand(Command command) {
        CompletableFuture<Command> promise = new CompletableFuture<>();
        CommandPromise commandPromise = new CommandPromise(command, promise);

        commandPromises.put(command.id, commandPromise);

        String message = command.toString();

        System.out.printf("< %s%n", message);

        messageTransport.sendMessage("%s\n", message);

        return promise;
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
        CommandPromise commandPromise = getCommandPromiseById(responseCommand.id);

        if (commandPromise == null) {
            System.out.printf("# original command promise for %s was not found%n", responseCommand.toString());

            return;
        }

        // System.out.printf("# got response %s for original command %s%n", responseCommand.toString(), commandPromise.command.toString());

        commandPromise.promise.complete(responseCommand);
    }

    private CommandPromise getCommandPromiseById(int id) {
        if (!commandPromises.containsKey(id)) {
            return null;
        }

        return commandPromises.get(id);
    }
}
