package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.List;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.devices.AbstractTagSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.PortController;

@Slf4j
public class EtherioTagSensor extends AbstractTagSensor {

	private final Commander commander;
	private final int portNumber;
	private PortController portController;

	private static final String CAPABILITY = "PN532";

	public EtherioTagSensor(String id, Commander commander, int portNumber) {
		super(id);

		this.commander = commander;
		this.portNumber = portNumber;
	}

	@Override
	public void initialize() throws Exception {
		super.initialize();

		portController = new PortController(portNumber, commander);
	}

	@Override
	public void start() {
		super.start();

		log.debug("starting tag sensor '{}'", id);

		commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to tag sensor '{}' commander has been {}, enabling it", id, isFirstConnect ? "established" : "re-established");

				if (isFirstConnect) {
					setupEventListener();
				}

				portController.sendPortCommand(CAPABILITY, "enable").thenAccept(commandResponse -> {
					log.debug("tag sensor '{}' has been enabled", id);

					state.setIsRunning(true);
					updateState(state);
				});
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.debug("tag sensor '{}' commander transport has been closed", id);

				state.setIsRunning(false);
				updateState(state);
			}
		});
	}

	@Override
	public void shutdown() {
		log.info("shutting down tag sensor '{}'", id);

		state.reset();
		updateState(state);

		super.shutdown();
	}

	private void setupEventListener() {
		portController.addEventListener(new PortController.PortEventListener() {
			@Override
			public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
				if (!capabilityName.equals(CAPABILITY)) {
					return;
				}

				String event = arguments.get(0);

				switch (event) {
					case "enter":
						emitTagEnter(arguments.get(1));
						break;

					case "exit":
						emitTagExit();
						break;

					case "uid":
						emitTagUidEvent(arguments.get(1), arguments.get(2));
						break;

					default:
						log.warn("unexpected tag event '{}' observed", event);
						break;
				}
			}
		});
	}

}
