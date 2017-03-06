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

	protected final MultiDacActuator state;

	protected AbstractMultiDacActuator(String id, int channelCount) {
		super(id);

		state = new MultiDacActuator();
		state.setChannelCount(channelCount);
	}

	@Override
	protected String getType() {
		return state.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return state;
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
		int channelCount = state.getChannelCount();

		log.info("starting, initializing the {} channels to off", channelCount);

		for (int i = 0; i < channelCount; i++) {
			state.updateChannelValue(i, 0.0f);
			applyChannelValue(i, 0.0f);
		}

		updateState(state);
	}
	*/

	public void setChannelValue(int channel, float value) {
		log.debug("setting channel {} value to {}", channel, value);

		applyChannelValue(channel, value);

		state.updateChannelValue(channel, value);
		updateState(state);
	}

	public void setChannelValues(Map<Integer, Float> values) {
		log.debug("setting multiple channel values");

		for (Map.Entry<Integer, Float> channelValuePair : values.entrySet()) {
			int channel = channelValuePair.getKey();
			float value = channelValuePair.getValue();

			log.debug("- {}: {}", channel, value);

			state.updateChannelValue(channel, value);
		}

		applyChannelValues(values);

		updateState(state);
	}

	protected abstract void applyChannelValue(int channel, float value);
	protected abstract void applyChannelValues(Map<Integer, Float> values);



}
