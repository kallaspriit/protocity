package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.cumulocity.model.measurement.StateType;
import com.cumulocity.model.measurement.ValueType;

import java.math.BigDecimal;

public class StateMeasurement<E extends Enum> {

    private MeasurementValue measurementValue;

    StateMeasurement() {
        this.measurementValue = new MeasurementValue(
                new BigDecimal(0),
                "state",
                ValueType.BOOLEAN,
                "",
                StateType.ORIGINAL
        );
    }

    StateMeasurement(E measurementValue) {
        this();

        setState(measurementValue);
    }

    @SuppressWarnings("unused")
    public MeasurementValue getState() {
        return measurementValue;
    }

    @SuppressWarnings("unused")
    public void setState(MeasurementValue measurementValue) {
        this.measurementValue = measurementValue;
    }

    public void setState(E state) {
        this.measurementValue.setValue(stateToValue(state));
    }

    private BigDecimal stateToValue(E state) {
        return new BigDecimal(state.ordinal());
    }

}
