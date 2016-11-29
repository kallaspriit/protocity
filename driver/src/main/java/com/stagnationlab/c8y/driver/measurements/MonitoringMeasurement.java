package com.stagnationlab.c8y.driver.measurements;

import java.math.BigDecimal;

import com.cumulocity.model.measurement.MeasurementValue;
import com.cumulocity.model.measurement.StateType;
import com.cumulocity.model.measurement.ValueType;

@SuppressWarnings({ "WeakerAccess", "unused" })
public class MonitoringMeasurement {

	private float totalMemory;
	private float usedMemory;
	private float freeMemory;
	private float sharedMemory;
	private float bufferedMemory;
	private float cachedMemory;

	private static final String UNIT = "bytes";

	MonitoringMeasurement(
			float totalMemory,
			float usedMemory,
			float freeMemory,
			float sharedMemory,
			float bufferedMemory,
			float cachedMemory
	) {
		this.totalMemory = totalMemory;
		this.usedMemory = usedMemory;
		this.freeMemory = freeMemory;
		this.sharedMemory = sharedMemory;
		this.bufferedMemory = bufferedMemory;
		this.cachedMemory = cachedMemory;
	}

	public MonitoringMeasurement(
			float totalMemory,
			float freeMemory
	) {
		this(
				totalMemory,
				totalMemory - freeMemory,
				freeMemory,
				-1,
				-1,
				-1
		);
	}

	public MeasurementValue getTotalMemory() {
		return buildMeasurementValue(totalMemory, UNIT);
	}

	public void setTotalMemory(MeasurementValue measurementValue) {
		totalMemory = measurementValue.getValue().floatValue();
	}

	public MeasurementValue getUsedMemory() {
		return buildMeasurementValue(usedMemory, UNIT);
	}

	public void setUsedMemory(MeasurementValue measurementValue) {
		usedMemory = measurementValue.getValue().floatValue();
	}

	public MeasurementValue getFreeMemory() {
		return buildMeasurementValue(freeMemory, UNIT);
	}

	public void setFreeMemory(MeasurementValue measurementValue) {
		freeMemory = measurementValue.getValue().floatValue();
	}

	public MeasurementValue getSharedMemory() {
		return buildMeasurementValue(sharedMemory, UNIT);
	}

	public void setSharedMemory(MeasurementValue measurementValue) {
		sharedMemory = measurementValue.getValue().floatValue();
	}

	public MeasurementValue getBufferedMemory() {
		return buildMeasurementValue(bufferedMemory, UNIT);
	}

	public void setBufferedMemory(MeasurementValue measurementValue) {
		bufferedMemory = measurementValue.getValue().floatValue();
	}

	public MeasurementValue getCachedMemory() {
		return buildMeasurementValue(cachedMemory, UNIT);
	}

	public void setCachedMemory(MeasurementValue measurementValue) {
		cachedMemory = measurementValue.getValue().floatValue();
	}

	@SuppressWarnings("SameParameterValue")
	private MeasurementValue buildMeasurementValue(float value, String unit) {
		return new MeasurementValue(
				new BigDecimal(value),
				unit,
				ValueType.GAUGE,
				"",
				StateType.ORIGINAL
		);
	}

}
