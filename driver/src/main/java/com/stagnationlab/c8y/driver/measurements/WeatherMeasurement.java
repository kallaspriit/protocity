package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.cumulocity.model.measurement.StateType;
import com.cumulocity.model.measurement.ValueType;

import java.math.BigDecimal;

@SuppressWarnings("unused")
public class WeatherMeasurement {

    private double temperature = 0;

    public void update(
            double temperature
    ) {
        this.temperature = temperature;
    }

    public MeasurementValue getTemperature() {
        return buildMeasurementValue(temperature, "°C");
    }

    public void setTemperature(MeasurementValue measurementValue) {
        temperature = measurementValue.getValue().doubleValue();
    }

    private MeasurementValue buildMeasurementValue(double value, String unit) {
        return new MeasurementValue(
                new BigDecimal(value),
                unit,
                ValueType.GAUGE,
                "",
                StateType.ORIGINAL
        );
    }

}
