package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings({ "WeakerAccess", "unused" })
public class TemperatureMeasurement {

	private float value;
	private final String unit;

	public TemperatureMeasurement(float value, String unit) {
		this.value = value;
		this.unit = unit;
	}

	public TemperatureMeasurement(float value) {
		this(value, "C");
	}

	public TemperatureMeasurement() {
		this(0.0f);
	}

	public MeasurementValue getTemperature() {
		return Util.buildMeasurementValue(value, unit);
	}

	public void setTemperature(MeasurementValue measurementValue) {
		value = measurementValue.getValue().floatValue();
	}

}
