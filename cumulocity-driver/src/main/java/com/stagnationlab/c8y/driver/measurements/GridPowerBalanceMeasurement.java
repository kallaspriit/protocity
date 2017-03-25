package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings("unused")
public class GridPowerBalanceMeasurement {

	private float gridPowerBalance;
	private String unit;

	public GridPowerBalanceMeasurement(float power, String unit) {
		this.gridPowerBalance = power;
		this.unit = unit;
	}

	public GridPowerBalanceMeasurement() {
		this(0.0f, "W");
	}

	public MeasurementValue getGridPowerBalance() {
		return Util.buildMeasurementValue(gridPowerBalance, unit);
	}

	public void setGridPowerBalance(MeasurementValue value) {
		this.gridPowerBalance = value.getValue().floatValue();
	}

}
