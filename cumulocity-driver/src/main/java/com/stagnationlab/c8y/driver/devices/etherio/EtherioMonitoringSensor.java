package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.concurrent.ScheduledFuture;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.devices.AbstractMonitoringSensor;
import com.stagnationlab.c8y.driver.fragments.sensors.MonitoringSensor;
import com.stagnationlab.c8y.driver.measurements.MonitoringMeasurement;
import com.stagnationlab.c8y.driver.services.Scheduler;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;

@Slf4j
public class EtherioMonitoringSensor extends AbstractMonitoringSensor {

	private final Commander commander;
	private ScheduledFuture<?> pollerInterval;

	private static final String COMMAND_GET_VERSION = "version";
	private static final String COMMAND_GET_MEMORY = "memory";
	private static final int POLL_INTERVAL = 60000;

	public EtherioMonitoringSensor(String id, Commander commander, String name, String host, int port) {
		super(id, name, host, port);

		this.commander = commander;
	}

	@Override
	public void start() {
		log.info("starting monitoring '{}'", id);

		commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to monitoring commander of '{}' has been {}", id, isFirstConnect ? "established" : "re-established");

				requestForVersion();
				startPoller();

				state.setState(MonitoringSensor.State.CONNECTED);
				updateState(state);
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.info("monitoring '{}' commander transport has been closed", id);

				stopPoller();

				state.setState(MonitoringSensor.State.DISCONNECTED);
				updateState(state);
			}

			@Override
			public void onConnecting(boolean isReconnecting) {
				state.setState(isReconnecting ? MonitoringSensor.State.RECONNECTING : MonitoringSensor.State.CONNECTING);
				updateState(state);
			}

			@Override
			public void onConnectionFailed(Exception e, boolean wasEverConnected) {
				if (wasEverConnected) {
					state.setState(MonitoringSensor.State.DISCONNECTED);
				} else {
					state.setState(MonitoringSensor.State.CONNECTION_FAILED);
				}

				updateState(state);
			}
		});
	}

	@Override
	public void shutdown() {
		super.shutdown();

		stopPoller();
	}

	private void requestForVersion() {
		log.debug("requesting for version of '{}'", id);

		commander.sendCommand(COMMAND_GET_VERSION).thenAccept(commandResponse -> {
			String version = commandResponse.response.getString(0);
			log.info("controller '{}' version: {}", id, version);

			state.setVersion(version);
			updateState(state);
		});
	}

	private void startPoller() {
		log.debug("starting poller for '{}'", id);

		pollerInterval = Scheduler.setInterval(this::poll, POLL_INTERVAL);
	}

	private void poll() {
		log.trace("polling for monitoring information for '{}'", id);

		commander.sendCommand(COMMAND_GET_MEMORY).thenAccept(
				commandResponse -> {
					int freeMemoryBytes = commandResponse.response.getInt(0);
					int initialMemoryBytes = commandResponse.response.getInt(1);
					int totalMemoryBytes = commandResponse.response.getInt(2);

					log.trace("'{}' free memory: {}/{} ({}%)", id, freeMemoryBytes, totalMemoryBytes, (float)freeMemoryBytes / (float)totalMemoryBytes * 100.0f);

					MonitoringMeasurement monitoringMeasurement = new MonitoringMeasurement(
							totalMemoryBytes,
							initialMemoryBytes,
							freeMemoryBytes
					);

					reportMeasurement(monitoringMeasurement);
				}
		);
	}

	private void stopPoller() {
		if (pollerInterval == null || pollerInterval.isDone() || pollerInterval.isCancelled()) {
			return;
		}

		log.debug("stopping poller for '{}'", id);

		pollerInterval.cancel(true);
	}
}
