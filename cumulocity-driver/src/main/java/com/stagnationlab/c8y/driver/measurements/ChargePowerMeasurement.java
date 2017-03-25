package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings("unused")
public class ChargePowerMeasurement {

	private float chargePower;
	private String unit;

	public ChargePowerMeasurement(float power, String unit) {
		this.chargePower = power;
		this.unit = unit;
	}

	public ChargePowerMeasurement() {
		this(0.0f, "W");
	}

	public MeasurementValue getChargePower() {
		return Util.buildMeasurementValue(chargePower, unit);
	}

	public void setChargePower(MeasurementValue value) {
		this.chargePower = value.getValue().floatValue();
	}

}
