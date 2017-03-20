package com.stagnationlab.c8y.driver.devices;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.fragments.sensors.AnalogInputSensor;
import com.stagnationlab.c8y.driver.measurements.AnalogMeasurement;

@Slf4j
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

	public AnalogInputSensor getState() {
		return state;
	}

	protected void handleValueChange(float value) {
		log.trace("analog input '{}' value changed to {}", id, value);

		state.setValue(value);
		updateState(state);

		sendMeasurement();
	}

	private void sendMeasurement() {
		AnalogMeasurement measurement = new AnalogMeasurement(state.getValue(), unit);

		reportMeasurement(measurement);
	}

}
