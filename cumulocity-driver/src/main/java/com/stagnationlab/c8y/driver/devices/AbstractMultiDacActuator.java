package com.stagnationlab.c8y.driver.devices;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.stagnationlab.c8y.driver.fragments.MultiDacActuator;
import com.stagnationlab.c8y.driver.operations.SetChannelValue;

import c8y.lx.driver.OperationExecutor;

public abstract class AbstractMultiDacActuator extends AbstractDevice {

	private static final Logger log = LoggerFactory.getLogger(AbstractMultiDacActuator.class);

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
				//return SetChannelValue.class.getSimpleName();
				//return "com_stagnationlab_c8y_driver_operations_SetChannelValue";

				return SetChannelValue.class.getCanonicalName().replaceAll("\\.", "_");
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

	public void setChannelValue(int channel, float value) {
		log.debug("setting channel {} value to {}", channel, value);

		applyChannelValue(channel, value);

		multiDacActuator.updateChannelValue(channel, value);
		updateState(multiDacActuator);
	}

	protected abstract void applyChannelValue(int channel, float value);



}
