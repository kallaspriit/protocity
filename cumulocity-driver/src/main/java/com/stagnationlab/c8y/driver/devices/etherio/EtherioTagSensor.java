package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.List;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.devices.AbstractTagSensor;
import com.stagnationlab.etherio.Commander;
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

		portController.sendPortCommand(CAPABILITY, "enable");

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

					default:
						log.warn("unexpected tag event '{}' observed", event);
						break;
				}
			}
		});
	}

}
