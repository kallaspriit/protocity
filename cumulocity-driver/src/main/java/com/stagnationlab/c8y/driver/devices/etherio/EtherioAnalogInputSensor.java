package com.stagnationlab.c8y.driver.devices.etherio;

import com.stagnationlab.c8y.driver.devices.AbstractAnalogInputSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class EtherioAnalogInputSensor extends AbstractAnalogInputSensor {

	private final Commander commander;
	private final int portNumber;
	private PortController portController;
	private static final float VALUE_MULTIPLIER = 100.0f;

	public EtherioAnalogInputSensor(String id, Commander commander, int portNumber, String unit) {
		super(id, unit);

		this.commander = commander;
		this.portNumber = portNumber;
	}

	@Override
	public void initialize() throws Exception {
		super.initialize();

		portController = new PortController(portNumber, commander);
		portController.setPortMode(PortController.PortMode.ANALOG_IN);
	}

	@Override
	public void start() {
		// listen for value changes
		portController.listenAnalogValueChange(0.01f, 500, new PortController.PortEventListener() {

			@Override
			public void onPortAnalogValueChange(int id, float value) {
				setValue(getTransformedValue(value));
			}
		});

		// also report periodically
		setInterval(() -> {
			portController.getAnalogValue().thenAccept(commandResponse -> {
				float currentValue = commandResponse.response.getFloat(0);

				setValue(getTransformedValue(currentValue));
			});
		}, 60000);
	}

	private float getTransformedValue(float value) {
		return Math.round(value * VALUE_MULTIPLIER);
	}

}
