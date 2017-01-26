package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.stagnationlab.c8y.driver.devices.AbstractTemperatureSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class EtherioTemperatureSensor extends AbstractTemperatureSensor {

	private static final Logger log = LoggerFactory.getLogger(EtherioTemperatureSensor.class);

	private final Commander commander;
	private final int portNumber;
	private PortController portController;

	private static final String CAPABILITY = "TMP102";

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

		portController.sendPortCommand(CAPABILITY, "enable", 5000);

		portController.addEventListener(new PortController.PortEventListener() {
			@Override
			public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
				if (!capabilityName.equals(CAPABILITY)) {
					return;
				}

				log.info("try to parse: {}", arguments.get(0));

				float temperature = Float.valueOf(arguments.get(0));

				log.info("reporting temperature: {}", temperature);

				reportTemperature(temperature);
			}
		});
	}

}
