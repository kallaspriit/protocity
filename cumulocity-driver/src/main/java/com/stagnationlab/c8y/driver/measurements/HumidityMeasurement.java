package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings({ "WeakerAccess", "unused" })
public class HumidityMeasurement {

	private float value;
	private String unit;

	public HumidityMeasurement(float value, String unit) {
		this.value = value;
		this.unit = unit;
	}

	public HumidityMeasurement(float value) {
		this(value, "%");
	}

	public HumidityMeasurement() {
		this(0.0f);
	}

	public MeasurementValue getHumidity() {
		return Util.buildMeasurementValue(value, unit);
	}

	public void setHumidity(MeasurementValue measurementValue) {
		value = measurementValue.getValue().floatValue();
	}

}
