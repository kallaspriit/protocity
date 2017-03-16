package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.List;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.Gateway;
import com.stagnationlab.c8y.driver.devices.AbstractTemperatureSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.PortController;

@SuppressWarnings("unused")
@Slf4j
public class EtherioTemperatureSensor extends AbstractTemperatureSensor {

	private final Commander commander;
	private final int portNumber;
	private PortController portController;

	private static final String TEMPERATURE_SENSOR_CAPABILITY = "TMP102";
	private static final String COMMAND_ENABLE = "enable";

	public EtherioTemperatureSensor(String id, Commander commander, int portNumber) {
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



		commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to temperature sensor '{}' commander has been {}, enabling it", id, isFirstConnect ? "established" : "re-established");

				if (isFirstConnect) {
					addEventListeners();
				}

				enableSensor();
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.debug("temperature sensor '{}' commander transport has been closed", id);
			}
		});
	}

	private void addEventListeners() {
		portController.addEventListener(new PortController.PortEventListener() {
			@Override
			public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
				if (!capabilityName.equals(TEMPERATURE_SENSOR_CAPABILITY)) {
					return;
				}

				log.info("try to parse: {}", arguments.get(0));

				float temperature = Float.valueOf(arguments.get(0));

				log.info("reporting temperature: {}", temperature);

				reportTemperature(temperature);
			}
		});
	}

	private void enableSensor() {
		portController.sendPortCommand(TEMPERATURE_SENSOR_CAPABILITY, COMMAND_ENABLE, 5000)
				.thenAccept(Gateway::handlePortCommandResponse);
	}

}
