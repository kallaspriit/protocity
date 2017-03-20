package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings("unused")
public class PowerMeasurement {

	private float power;
	private String unit;

	public PowerMeasurement(float power, String unit) {
		this.power = power;
		this.unit = unit;
	}

	public PowerMeasurement() {
		this(0.0f, "W");
	}

	public MeasurementValue getPower() {
		return Util.buildMeasurementValue(power, unit);
	}

	public void setPower(MeasurementValue value) {
		this.power = value.getValue().floatValue();
	}

}
