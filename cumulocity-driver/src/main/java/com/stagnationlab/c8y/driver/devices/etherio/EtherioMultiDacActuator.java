package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.PortController;

@Slf4j
public class EtherioMultiDacActuator extends AbstractMultiDacActuator {

	private final Commander commander;
	private final int portNumber;
	private PortController portController;
	private boolean isActive = false;

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
	}

	@Override
	public void start() {
		super.start();

		commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to multi-dac actuator '{}' commander has been {}, enabling it", id, isFirstConnect ? "established" : "re-established");

				isActive = true;

				portController.sendPortCommand(CAPABILITY, "enable");
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.debug("multi-dac actuator '{}' commander transport has been closed", id);

				isActive = false;
			}
		});
	}

	@Override
	protected void applyChannelValue(int channel, float value) {
		if (!isActive) {
			log.warn("setting channel {} value to {} failed, device is not active", channel, value);

			return;
		}

		portController.sendPortCommand(CAPABILITY, "value", channel, value);
	}

	@Override
	protected void applyChannelValues(Map<Integer, Float> values) {
		if (!isActive) {
			log.warn("setting multiple channel values failed, device is not active");

			return;
		}

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
