package com.stagnationlab.c8y.driver.services;

import java.math.BigDecimal;
import java.util.Calendar;

import org.svenson.JSON;

import com.cumulocity.model.measurement.MeasurementValue;
import com.cumulocity.model.measurement.StateType;
import com.cumulocity.model.measurement.ValueType;

public final class Util {

	private Util() {}

    public static double map(double value, double inMin, double inMax, double outMin, double outMax) {
        return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    }

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

	public static MeasurementValue buildMeasurementValue(double value, String unit) {
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

}
