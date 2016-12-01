package com.stagnationlab.c8y.driver.platforms.etherio;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.stagnationlab.c8y.driver.devices.AbstractMotionSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class EtherioMotionSensor extends AbstractMotionSensor {

	private static final Logger log = LoggerFactory.getLogger(EtherioMotionSensor.class);

	private final Commander commander;
	private final int portNumber;
	private PortController portController;

	public EtherioMotionSensor(String id, Commander commander, int portNumber) {
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
				log.info("motion port {} changed to {}", id, digitalValue.name());

				setIsMotionDetected(digitalValue == PortController.DigitalValue.LOW);
			}
		});
		portController.setPullMode(PortController.PullMode.UP);
	}

}
