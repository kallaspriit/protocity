package com.stagnationlab.c8y.driver.services;

import java.math.BigDecimal;
import java.util.Calendar;
import java.util.Random;

import org.svenson.JSON;

import com.cumulocity.model.measurement.MeasurementValue;
import com.cumulocity.model.measurement.StateType;
import com.cumulocity.model.measurement.ValueType;

public final class Util {

	private static Random random = new Random();

	private Util() {}

    public static String stringify(Object obj) {
        return JSON.defaultJSON().forValue(obj);
    }

	@SuppressWarnings("SameParameterValue")
	public static MeasurementValue buildMeasurementValue(float value, String unit) {
		return new MeasurementValue(
				new BigDecimal(value),
				unit,
				ValueType.GAUGE,
				"",
				StateType.ORIGINAL
		);
	}

	public static String buildOperationName(Class reference) {
    	return reference.getCanonicalName().replaceAll("\\.", "_");
	}

	public static float map(float value, float inMin, float inMax, float outMin, float outMax) {
    	if (outMax < outMin) {
    		return map(value, inMax, inMin, outMax, outMin);
	    }

		float result = (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;

		if (result < outMin) {
			result = outMin;
		} else if (result > outMax) {
			result = outMax;
		}

		return result;
	}

	public static long now() {
		return Calendar.getInstance().getTimeInMillis();
	}

	public static long since(long millis) {
    	return now() - millis;
	}

	public static float round(float value, int decimalPlaces) {
		float factor = (float)Math.pow(10.0, decimalPlaces);

		return (float)Math.round(value * factor) / factor;
	}

	// http://stackoverflow.com/a/9723994
	public static float getRandomFloatInRange(float min, float max) {
		float range = max - min;
		float scaled = random.nextFloat() * range;

		return scaled + min;
	}

}
