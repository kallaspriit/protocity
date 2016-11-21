package com.stagnationlab.etherio;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;

@SuppressWarnings({"WeakerAccess", "unused"})
public class PortController implements MessageTransport.MessageListener {

    public enum PortMode {
        UNUSED,
        INVALID,
        OUTPUT,
        INPUT,
        INTERRUPT,
        PWM,
        ANALOG
    }

    public enum DigitalValue {
        LOW,
        HIGH
    }

    public interface PortEventListener {
        void onPortDigitalValueChange(int id, DigitalValue value);
        void onPortAnalogValueChange(int id, float value);
        void onPortValueRise(int id);
        void onPortValueFall(int id);
    }

    public class CommandResponse {
        public final Command command;
        public Command response;

        CommandResponse(Command command, Command response) {
            this.command = command;
            this.response = response;
        }

        CommandResponse(Command command) {
            this(command, null);
        }
    }

    private class CommandPromise {
        final CommandResponse commandResponse;
        final CompletableFuture<CommandResponse> promise;

        CommandPromise(CommandResponse commandResponse, CompletableFuture<CommandResponse> promise) {
            this.commandResponse = commandResponse;
            this.promise = promise;
        }
    }

    private final MessageTransport messageTransport;
    private final List<PortEventListener> portEventListeners;
    private final Map<Integer, CommandPromise> commandPromises;

    public PortController(MessageTransport messageTransport) {
        this.messageTransport = messageTransport;
        this.portEventListeners = new ArrayList<>();
        this.commandPromises = new HashMap<>();

        messageTransport.addMessageListener(this);
    }

    public void addEventListener(PortEventListener listener) {
        portEventListeners.add(listener);
    }

    public CompletableFuture<CommandResponse> sendCommand(String name, Object... arguments) {
        Command command = new Command(messageTransport.getNextMessageId(), name, arguments);

        return sendCommand(command);
    }

    public CompletableFuture<CommandResponse> sendCommand(Command command) {
        CompletableFuture<CommandResponse> promise = new CompletableFuture<>();
        CommandResponse commandResponse = new CommandResponse(command);
        CommandPromise commandPromise = new CommandPromise(commandResponse, promise);

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

        commandPromise.commandResponse.response = responseCommand;
        commandPromise.promise.complete(commandPromise.commandResponse);
        commandPromises.remove(responseCommand.id);
    }

    private CommandPromise getCommandPromiseById(int id) {
        if (!commandPromises.containsKey(id)) {
            return null;
        }

        return commandPromises.get(id);
    }
}
