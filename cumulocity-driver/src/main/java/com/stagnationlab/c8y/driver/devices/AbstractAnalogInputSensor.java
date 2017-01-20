package com.stagnationlab.c8y.driver.devices;

import com.stagnationlab.c8y.driver.fragments.AnalogInputSensor;
import com.stagnationlab.c8y.driver.measurements.AnalogMeasurement;

public class AbstractAnalogInputSensor extends AbstractDevice {

	private final AnalogInputSensor analogInputSensor = new AnalogInputSensor();
	private String unit;

	protected AbstractAnalogInputSensor(String id, String unit) {
		super(id);

		this.unit = unit;
	}

	@Override
	protected String getType() {
		return analogInputSensor.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return analogInputSensor;
	}

	protected void setValue(float value) {
		analogInputSensor.setValue(value);

		updateState(analogInputSensor);
		sendMeasurement();
	}

	private void sendMeasurement() {
		AnalogMeasurement measurement = new AnalogMeasurement(analogInputSensor.getValue(), unit);

		reportMeasurement(measurement);
	}

}
