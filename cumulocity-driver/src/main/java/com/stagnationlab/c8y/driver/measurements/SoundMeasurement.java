package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.stagnationlab.c8y.driver.services.Util;

@SuppressWarnings({ "WeakerAccess", "unused" })
public class SoundMeasurement {

	private float value;
	private String unit;

	public SoundMeasurement(float value, String unit) {
		this.value = value;
		this.unit = unit;
	}

	public SoundMeasurement(float value) {
		this(value, "dB");
	}

	public SoundMeasurement() {
		this(0.0f);
	}

	public MeasurementValue getSoundLevel() {
		return Util.buildMeasurementValue(value, unit);
	}

	public void setSoundLevel(MeasurementValue measurementValue) {
		value = measurementValue.getValue().floatValue();
	}

}