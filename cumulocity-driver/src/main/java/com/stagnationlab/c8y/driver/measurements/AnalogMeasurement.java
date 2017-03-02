package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

public class AnalogMeasurement {

	private float value;
	private final String unit;

	@SuppressWarnings("unused")
	public AnalogMeasurement() {
		value = 0.0f;
		unit = "";
	}

	public AnalogMeasurement(float value, String unit) {
		this.value = value;
		this.unit = unit;
	}

	public MeasurementValue getValue() {
		return Util.buildMeasurementValue(value, unit);
	}

	public void setValue(MeasurementValue measurementValue) {
		value = measurementValue.getValue().floatValue();
	}

}
