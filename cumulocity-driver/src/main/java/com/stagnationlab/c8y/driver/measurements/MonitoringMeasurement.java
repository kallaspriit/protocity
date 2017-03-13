package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings({ "WeakerAccess", "unused" })
public class MonitoringMeasurement {

	private int totalMemory;
	private int initialMemory;
	private int usedMemory;
	private int freeMemory;
	private float freeMemoryPercentage;

	private static final String UNIT = "bytes";

	public MonitoringMeasurement(
			int totalMemory,
			int initialMemory,
			int freeMemory
	) {
		this.totalMemory = totalMemory;
		this.initialMemory = initialMemory;
		this.usedMemory = totalMemory - freeMemory;
		this.freeMemory = freeMemory;
		this.freeMemoryPercentage = (float)freeMemory / (float)totalMemory * 100.0f;
	}

	public MeasurementValue getTotalMemory() {
		return Util.buildMeasurementValue(totalMemory, UNIT);
	}

	public void setTotalMemory(MeasurementValue measurementValue) {
		totalMemory = measurementValue.getValue().intValue();
	}

	public MeasurementValue getInitialMemory() {
		return Util.buildMeasurementValue(initialMemory, UNIT);
	}

	public void setInitialMemory(MeasurementValue measurementValue) {
		initialMemory = measurementValue.getValue().intValue();
	}

	public MeasurementValue getUsedMemory() {
		return Util.buildMeasurementValue(usedMemory, UNIT);
	}

	public void setUsedMemory(MeasurementValue measurementValue) {
		usedMemory = measurementValue.getValue().intValue();
	}

	public MeasurementValue getFreeMemory() {
		return Util.buildMeasurementValue(freeMemory, UNIT);
	}

	public void setFreeMemory(MeasurementValue measurementValue) {
		freeMemory = measurementValue.getValue().intValue();
	}

	public MeasurementValue getFreeMemoryPercentage() {
		return Util.buildMeasurementValue(freeMemoryPercentage, "%");
	}

	public void setFreeMemoryPercentage(MeasurementValue measurementValue) {
		freeMemoryPercentage = measurementValue.getValue().floatValue();
	}
}
