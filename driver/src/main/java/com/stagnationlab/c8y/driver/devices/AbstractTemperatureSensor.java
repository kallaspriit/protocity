package com.stagnationlab.c8y.driver.devices;

import com.stagnationlab.c8y.driver.fragments.TemperatureSensor;
import com.stagnationlab.c8y.driver.measurements.TemperatureMeasurement;

public abstract class AbstractTemperatureSensor extends AbstractDevice {

    private final TemperatureSensor temperatureSensor = new TemperatureSensor();

	protected AbstractTemperatureSensor(String id) {
		super(id);
	}

	@Override
	protected String getType() {
		return temperatureSensor.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return temperatureSensor;
	}

	protected void reportTemperature(float temperature) {
		TemperatureMeasurement temperatureMeasurement = new TemperatureMeasurement(temperature);

		reportMeasurement(temperatureMeasurement);
	}

}
