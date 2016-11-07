package com.stagnationlab.c8y.driver.measurements;

import com.cumulocity.model.measurement.MeasurementValue;
import com.cumulocity.model.measurement.StateType;
import com.cumulocity.model.measurement.ValueType;

import java.math.BigDecimal;

@SuppressWarnings("unused")
public class DeviceMonitoringMeasurement {

    private double totalMemory = 0;
    private double usedMemory = 0;
    private double freeMemory = 0;
    private double sharedMemory = 0;
    private double bufferedMemory = 0;
    private double cachedMemory = 0;

    public void update(
            double totalMemory,
            double usedMemory,
            double freeMemory,
            double sharedMemory,
            double bufferedMemory,
            double cachedMemory
    ) {
        this.totalMemory = totalMemory;
        this.usedMemory = usedMemory;
        this.freeMemory = freeMemory;
        this.sharedMemory = sharedMemory;
        this.bufferedMemory = bufferedMemory;
        this.cachedMemory = cachedMemory;
    }

    public MeasurementValue getTotalMemory() {
        return buildMeasurementValue(totalMemory, "MB");
    }

    public void setTotalMemory(MeasurementValue measurementValue) {
        totalMemory = measurementValue.getValue().doubleValue();
    }

    public MeasurementValue getUsedMemory() {
        return buildMeasurementValue(usedMemory, "MB");
    }

    public void setUsedMemory(MeasurementValue measurementValue) {
        usedMemory = measurementValue.getValue().doubleValue();
    }

    public MeasurementValue getFreeMemory() {
        return buildMeasurementValue(freeMemory, "MB");
    }

    public void setFreeMemory(MeasurementValue measurementValue) {
        freeMemory = measurementValue.getValue().doubleValue();
    }

    public MeasurementValue getSharedMemory() {
        return buildMeasurementValue(sharedMemory, "MB");
    }

    public void setSharedMemory(MeasurementValue measurementValue) {
        sharedMemory = measurementValue.getValue().doubleValue();
    }

    public MeasurementValue getBufferedMemory() {
        return buildMeasurementValue(bufferedMemory, "MB");
    }

    public void setBufferedMemory(MeasurementValue measurementValue) {
        bufferedMemory = measurementValue.getValue().doubleValue();
    }

    public MeasurementValue getCachedMemory() {
        return buildMeasurementValue(cachedMemory, "MB");
    }

    public void setCachedMemory(MeasurementValue measurementValue) {
        cachedMemory = measurementValue.getValue().doubleValue();
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
