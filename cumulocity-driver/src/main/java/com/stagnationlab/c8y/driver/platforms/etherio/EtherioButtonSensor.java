package com.stagnationlab.c8y.driver.platforms.etherio;

import com.stagnationlab.c8y.driver.devices.AbstractButtonSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class EtherioButtonSensor extends AbstractButtonSensor {

	private final Commander commander;
	private final int portNumber;
	private PortController portController;

	public EtherioButtonSensor(String id, Commander commander, int portNumber) {
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
		portController.setPortMode(PortController.PortMode.INTERRUPT, new PortController.PortEventListener() {

			@Override
			public void onPortDigitalValueChange(int id, PortController.DigitalValue digitalValue) {
				setIsButtonPressed(digitalValue == PortController.DigitalValue.HIGH);
			}
		});
	}

}
