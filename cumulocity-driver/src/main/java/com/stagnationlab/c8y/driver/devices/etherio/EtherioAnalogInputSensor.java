package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.concurrent.ScheduledFuture;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.Gateway;
import com.stagnationlab.c8y.driver.devices.AbstractAnalogInputSensor;
import com.stagnationlab.c8y.driver.services.Util;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.PortController;

@Slf4j
public class EtherioAnalogInputSensor extends AbstractAnalogInputSensor {

	private final Commander commander;
	private final int portNumber;
	private PortController portController;
	private ScheduledFuture<?> pollerInterval;

	private static final int MIN_INTERVAL_MS = 500;
	private static final float VALUE_MULTIPLIER = 100.0f;
	private static final float CHANGE_THRESHOLD = 0.01f;
	private static final int POLL_INTERVAL = 60000;
	private static final String RESPONSE_OK = "OK";

	public EtherioAnalogInputSensor(String id, Commander commander, int portNumber, String unit) {
		super(id, unit);

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

		log.debug("starting analog input sensor '{}'", id);

		commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to tag sensor '{}' commander has been {}, enabling it", id, isFirstConnect ? "established" : "re-established");

				// listen for port analog value changes
				if (isFirstConnect) {
					portController.addEventListener(new PortController.PortEventListener() {
						@Override
						public void onPortAnalogValueChange(int id, float value) {
							handleValueChange(getTransformedValue(value));
						}
					});
				}

				// use analog input
				portController.setPortMode(PortController.PortMode.ANALOG_IN).thenAccept(commandResponse -> {
					if (commandResponse.response.name.equals(RESPONSE_OK)) {
						log.debug("analog input sensor '{}' has been enabled", id);

						state.setIsRunning(true);
						updateState(state);

						startValueListener();
						startPoller();
					}

					Gateway.handlePortCommandResponse(commandResponse);
				});
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.debug("analog input sensor '{}' commander transport has been closed", id);

				stopPoller();

				state.setIsRunning(false);
				updateState(state);
			}
		});
	}

	@Override
	public void shutdown() {
		super.shutdown();

		stopPoller();
	}

	private void startValueListener() {
		portController.listenAnalogValueChange(CHANGE_THRESHOLD, MIN_INTERVAL_MS);
	}

	private void startPoller() {
		log.debug("starting poller for '{}'", id);

		pollerInterval = setInterval(this::poll, POLL_INTERVAL);
	}

	private void poll() {
		log.trace("polling for analog value for '{}'", id);

		portController.getAnalogValue().thenAccept(commandResponse -> {
			float currentValue = commandResponse.response.getFloat(0);

			handleValueChange(getTransformedValue(currentValue));
		});
	}

	private void stopPoller() {
		if (pollerInterval == null || pollerInterval.isDone() || pollerInterval.isCancelled()) {
			return;
		}

		log.debug("stopping poller for '{}'", id);

		pollerInterval.cancel(true);
	}

	private float getTransformedValue(float value) {
		return Util.round(value * VALUE_MULTIPLIER, 1);
	}

}
