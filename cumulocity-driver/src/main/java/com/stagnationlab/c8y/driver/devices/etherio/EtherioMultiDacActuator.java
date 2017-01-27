package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.Map;

import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class EtherioMultiDacActuator extends AbstractMultiDacActuator {

	private final Commander commander;
	private final int portNumber;
	private PortController portController;

	private static final String CAPABILITY = "TLC5940";

	public EtherioMultiDacActuator(String id, Commander commander, int portNumber, int channelCount) {
		super(id, channelCount);

		this.commander = commander;
		this.portNumber = portNumber;
	}

	@Override
	public void initialize() throws Exception {
		super.initialize();

		portController = new PortController(portNumber, commander);

		portController.sendPortCommand(CAPABILITY, "enable");
	}

	@Override
	protected void applyChannelValue(int channel, float value) {
		portController.sendPortCommand(CAPABILITY, "value", channel, value);
	}

	@Override
	protected void applyChannelValues(Map<Integer, Float> values) {
		String valuesArg = "";
		boolean isFirst = true;

		for (int channel : values.keySet()) {
			float value = values.get(channel);

			if (!isFirst) {
				valuesArg += ",";
			}

			valuesArg += channel + "-" + value;

			isFirst = false;
		}

		portController.sendPortCommand(CAPABILITY, "values", valuesArg);
	}
}
