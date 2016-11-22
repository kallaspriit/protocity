package com.stagnationlab.etherio;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CompletableFuture;

@SuppressWarnings({"WeakerAccess", "unused"})
public class Commander implements MessageTransport.MessageListener {

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
    private final Map<Integer, CommandPromise> commandPromises;

    public Commander(MessageTransport messageTransport) {
        this.messageTransport = messageTransport;
        this.commandPromises = new HashMap<>();

        messageTransport.addMessageListener(this);
    }

    public CompletableFuture<Commander.CommandResponse> sendCommand(String name, Object... arguments) {
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

            handleResponse(responseCommand);
        } catch (Exception e) {
            System.out.printf("# got invalid response '%s'%n", message);
        }
    }

    private void handleResponse(Command responseCommand) {
        CommandPromise commandPromise = getCommandPromiseById(responseCommand.id);

        if (commandPromise == null) {
            // System.out.printf("# original command promise for %s was not found%n", responseCommand.toString());

            return;
        }

        System.out.printf("> %s%n", responseCommand.toString());

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
