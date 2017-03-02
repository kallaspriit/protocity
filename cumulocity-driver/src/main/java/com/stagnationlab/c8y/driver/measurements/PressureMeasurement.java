package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings({ "WeakerAccess", "unused" })
public class PressureMeasurement {

	private float value;
	private final String unit;

	public PressureMeasurement(float value, String unit) {
		this.value = value;
		this.unit = unit;
	}

	public PressureMeasurement(float value) {
		this(value, "%");
	}

	public PressureMeasurement() {
		this(0.0f, "mmHg");
	}

	public MeasurementValue getPressure() {
		return Util.buildMeasurementValue(value, unit);
	}

	public void setPressure(MeasurementValue measurementValue) {
		value = measurementValue.getValue().floatValue();
	}

}