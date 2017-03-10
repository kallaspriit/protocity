package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings("unused")
public class BatteryMeasurement {

	private float voltage;
	private int chargePercentage;
	private boolean isCharging;

	public BatteryMeasurement(float voltage, int chargePercentage, boolean isCharging) {
		this.voltage = voltage;
		this.chargePercentage = chargePercentage;
		this.isCharging = isCharging;
	}

	public BatteryMeasurement() {
		this(0.0f, 0, false);
	}

	public MeasurementValue getVoltage() {
		return Util.buildMeasurementValue(voltage, "V");
	}

	public void setVoltage(MeasurementValue value) {
		this.voltage = value.getValue().floatValue();
	}

	public MeasurementValue getChargePercentage() {
		return Util.buildMeasurementValue(chargePercentage, "%");
	}

	public void setChargePercentage(MeasurementValue value) {
		this.chargePercentage = value.getValue().intValue();
	}

	public MeasurementValue getIsCharging() {
		return Util.buildMeasurementValue(isCharging ? 1 : 0, "state");
	}

	public void setIsCharging(MeasurementValue value) {
		this.isCharging = value.getValue().intValue() == 1 ? true : false;
	}
}
