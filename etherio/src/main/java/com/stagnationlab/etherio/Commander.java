package com.stagnationlab.etherio;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;

import lombok.extern.slf4j.Slf4j;

@SuppressWarnings({ "WeakerAccess", "unused" })
@Slf4j
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

	public interface SpecialCommandListener {
		void handleSpecialCommand(Command command);
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
	private final List<SpecialCommandListener> specialCommandListeners;

	public Commander(MessageTransport messageTransport) {
		this.messageTransport = messageTransport;
		this.commandPromises = new HashMap<>();
		this.specialCommandListeners = new ArrayList<>();

		messageTransport.addMessageListener(this);
	}

	public void addSpecialCommandListener(SpecialCommandListener listener) {
		specialCommandListeners.add(listener);
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

		log.debug("< {}", message);

		messageTransport.sendMessage("%s\n", message);

		return promise;
	}

	@Override
	public void onSocketConnecting(boolean isReconnecting) {
		log.info("connecting to socket");
	}

	@Override
	public void onSocketOpen() {
		log.info("socket connection opened");
	}

	@Override
	public void onSocketClose() {
		log.warn("socket connection closed");
	}

	@Override
	public void onSocketMessageReceived(String message) {
		Command responseCommand;

		try {
			responseCommand = Command.parse(message);
		} catch (Exception e) {
			log.warn("got invalid response '{}' ({})", message, e.getMessage());

			return;
		}

		try {
			handleResponse(responseCommand);
		} catch (Exception e) {
			log.warn("handling command '{}' failed ({} - {})", responseCommand.toString(), e.getClass().getSimpleName(), e.getMessage());
		}
	}

	@Override
	public void onSocketConnectionFailed(Exception e) {
		log.warn("connecting to socket failed ({} - {})", e.getClass().getSimpleName(), e.getMessage());
	}

	private void handleResponse(Command responseCommand) {
		if (responseCommand.id == 0) {
			handleSpecialCommand(responseCommand);
		}

		CommandPromise commandPromise = getCommandPromiseById(responseCommand.id);

		if (commandPromise == null) {
			return;
		}

		log.debug("> {}", responseCommand.toString());

		commandPromise.commandResponse.response = responseCommand;
		commandPromise.promise.complete(commandPromise.commandResponse);
		commandPromises.remove(responseCommand.id);
	}

	private void handleSpecialCommand(Command responseCommand) {
		synchronized (this) {
			for (SpecialCommandListener listener : specialCommandListeners) {
				listener.handleSpecialCommand(responseCommand);
			}
		}
	}

	private CommandPromise getCommandPromiseById(int id) {
		if (!commandPromises.containsKey(id)) {
			return null;
		}

		return commandPromises.get(id);
	}

}
