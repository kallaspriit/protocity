package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.concurrent.ScheduledFuture;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.Gateway;
import com.stagnationlab.c8y.driver.devices.AbstractAnalogInputSensor;
import com.stagnationlab.c8y.driver.services.Scheduler;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.PortController;

@Slf4j
public class EtherioAnalogInputSensor extends AbstractAnalogInputSensor {

	private final Commander commander;
	private final int portNumber;
	private PortController portController;
	private ScheduledFuture<?> pollerInterval;
	private final int minInterval;
	private final int pollInterval;
	private final float changeThreshold;

	private static final int DEFAULT_MIN_INTERVAL_MS = 500;
	private static final int DEFAULT_POLL_INTERVAL = 60000;
	private static final float DEFAULT_CHANGE_THRESHOLD = 0.01f;
	private static final float VALUE_MULTIPLIER = 100.0f;
	private static final String RESPONSE_OK = "OK";

	@SuppressWarnings("unused")
	public EtherioAnalogInputSensor(String id, Commander commander, int portNumber, String unit, int minInterval, int pollInterval, float changeThreshold) {
		super(id, unit);

		this.commander = commander;
		this.portNumber = portNumber;
		this.minInterval = minInterval;
		this.pollInterval = pollInterval;
		this.changeThreshold = changeThreshold;
	}

	@SuppressWarnings("unused")
	public EtherioAnalogInputSensor(String id, Commander commander, int portNumber, String unit) {
		this(id, commander, portNumber, unit, DEFAULT_MIN_INTERVAL_MS, DEFAULT_POLL_INTERVAL, DEFAULT_CHANGE_THRESHOLD);
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
		log.debug("starting value listener for '{}' with change threshold of {} and minimum reporting interval of {}ms", id, changeThreshold, minInterval);

		portController.listenAnalogValueChange(changeThreshold, minInterval);
	}

	private void startPoller() {
		log.debug("starting poller for '{}' every {}ms", id, pollInterval);

		pollerInterval = Scheduler.setInterval(this::poll, pollInterval);
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
		return value * VALUE_MULTIPLIER;
	}

}
