package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings({ "WeakerAccess", "unused" })
public class LightMeasurement {

	private float value;
	private String unit;

	public LightMeasurement() {
		value = 0.0f;
		unit = "lux";
	}

	public LightMeasurement(float value, String unit) {
		this.value = value;
		this.unit = unit;
	}

	public LightMeasurement(float value) {
		this(value, "lux");
	}

	public MeasurementValue getLuminosity() {
		return Util.buildMeasurementValue(value, unit);
	}

	public void setLuminosity(MeasurementValue measurementValue) {
		value = measurementValue.getValue().floatValue();
	}

}
