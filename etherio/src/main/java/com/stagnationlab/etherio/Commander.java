package com.stagnationlab.etherio;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;

import lombok.extern.slf4j.Slf4j;

@SuppressWarnings({ "WeakerAccess", "unused" })
@Slf4j
public class Commander implements MessageTransport.EventListener {

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

	public interface RemoteCommandListener {
		void handleRemoteCommand(Command command);
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
	private final List<RemoteCommandListener> remoteCommandListeners;

	public Commander(MessageTransport messageTransport) {
		this.messageTransport = messageTransport;
		this.commandPromises = new HashMap<>();
		this.remoteCommandListeners = new ArrayList<>();

		messageTransport.addEventListener(this);
	}

	public void addRemoteCommandListener(RemoteCommandListener listener) {
		remoteCommandListeners.add(listener);
	}

	public CompletableFuture<Commander.CommandResponse> sendCommand(String name, Object... arguments) {
		Command command = new Command(messageTransport.getNextMessageId(), name, arguments);

		return sendCommand(command);
	}

	public CompletableFuture<CommandResponse> sendCommand(Command command) {
		String message = command.toString();

		if (!messageTransport.isConnected()) {
			log.warn("sending message '{}' requested but message transport is not connected", message);

			CompletableFuture<CommandResponse> promise = new CompletableFuture<>();
			promise.cancel(false);

			return promise;
		}

		CompletableFuture<CommandResponse> promise = new CompletableFuture<>();
		CommandResponse commandResponse = new CommandResponse(command);
		CommandPromise commandPromise = new CommandPromise(commandResponse, promise);

		commandPromises.put(command.id, commandPromise);

		log.debug("< {}", message);

		messageTransport.sendMessage("%s\n", message);

		return promise;
	}

	@Override
	public void onConnecting(boolean isReconnecting) {
		log.debug("{} to socket", isReconnecting ? "reconnecting" : "connecting");
	}

	@Override
	public void onOpen(boolean wasReconnected) {
		log.debug("{} to socket", wasReconnected ? "reconnected" : "connected");
	}

	@Override
	public void onClose() {
		log.debug("socket connection closed");
	}

	@Override
	public void onMessageReceived(String message) {
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
	public void onConnectionFailed(Exception e, boolean wasEverOpened) {
		if (wasEverOpened) {
			log.debug("reconnecting to socket failed ({} - {})", e.getClass().getSimpleName(), e.getMessage());
		} else {
			log.warn("connecting to socket failed ({} - {})", e.getClass().getSimpleName(), e.getMessage());
		}
	}

	public MessageTransport getMessageTransport() {
		return messageTransport;
	}

	public boolean isConnected() {
		return messageTransport.isConnected();
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
			for (RemoteCommandListener listener : remoteCommandListeners) {
				listener.handleRemoteCommand(responseCommand);
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
