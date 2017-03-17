package com.stagnationlab.c8y.driver.devices;

import com.stagnationlab.c8y.driver.fragments.sensors.AnalogInputSensor;
import com.stagnationlab.c8y.driver.measurements.AnalogMeasurement;

public class AbstractAnalogInputSensor extends AbstractDevice {

	protected final AnalogInputSensor state = new AnalogInputSensor();
	protected final String unit;

	protected AbstractAnalogInputSensor(String id, String unit) {
		super(id);

		this.unit = unit;
	}

	@Override
	protected String getType() {
		return state.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return state;
	}

	protected void handleValueChange(float value) {
		state.setValue(value);
		updateState(state);

		sendMeasurement();
	}

	private void sendMeasurement() {
		AnalogMeasurement measurement = new AnalogMeasurement(state.getValue(), unit);

		reportMeasurement(measurement);
	}

}
