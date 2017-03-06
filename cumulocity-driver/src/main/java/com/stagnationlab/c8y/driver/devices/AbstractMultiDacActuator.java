package com.stagnationlab.c8y.driver.devices;

import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.stagnationlab.c8y.driver.fragments.actuators.MultiDacActuator;
import com.stagnationlab.c8y.driver.operations.SetChannelValue;
import com.stagnationlab.c8y.driver.services.Util;

import c8y.lx.driver.OperationExecutor;

@Slf4j
public abstract class AbstractMultiDacActuator extends AbstractDevice {

	private final MultiDacActuator multiDacActuator;

	protected AbstractMultiDacActuator(String id, int channelCount) {
		super(id);

		multiDacActuator = new MultiDacActuator(channelCount);
	}

	@Override
	protected String getType() {
		return multiDacActuator.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return multiDacActuator;
	}

	@Override
	public void initialize() throws Exception {
		registerOperationExecutor(new OperationExecutor() {
			@Override
			public String supportedOperationType() {
				return Util.buildOperationName(SetChannelValue.class);
			}

			@Override
			public void execute(OperationRepresentation operation, boolean cleanup) throws Exception {
				if (!device.getId().equals(operation.getDeviceId())) {
					return;
				}

				if (cleanup) {
					log.info("ignoring cleanup operation");

					operation.setStatus(OperationStatus.FAILED.toString());

					return;
				}

				SetChannelValue action = operation.get(SetChannelValue.class);

				if (action == null) {
					log.warn("operation is missing the SetChannelValue object");

					return;
				}

				log.info("got operation request to set channel {} to {}", action.getChannel(), action.getValue());

				setChannelValue(action.getChannel(), action.getValue());

				operation.setStatus(OperationStatus.SUCCESSFUL.toString());
			}
		});
	}

	/*
	@Override
	public void start() {
		int channelCount = multiDacActuator.getChannelCount();

		log.info("starting, initializing the {} channels to off", channelCount);

		for (int i = 0; i < channelCount; i++) {
			multiDacActuator.updateChannelValue(i, 0.0f);
			applyChannelValue(i, 0.0f);
		}

		updateState(multiDacActuator);
	}
	*/

	public void setChannelValue(int channel, float value) {
		log.debug("setting channel {} value to {}", channel, value);

		applyChannelValue(channel, value);

		multiDacActuator.updateChannelValue(channel, value);
		updateState(multiDacActuator);
	}

	public void setChannelValues(Map<Integer, Float> values) {
		log.debug("setting multiple channel values");

		for (Map.Entry<Integer, Float> channelValuePair : values.entrySet()) {
			int channel = channelValuePair.getKey();
			float value = channelValuePair.getValue();

			log.debug("- {}: {}", channel, value);

			multiDacActuator.updateChannelValue(channel, value);
		}

		applyChannelValues(values);

		updateState(multiDacActuator);
	}

	protected abstract void applyChannelValue(int channel, float value);
	protected abstract void applyChannelValues(Map<Integer, Float> values);



}
