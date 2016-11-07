package com.stagnationlab.c8y.driver.sensors;

import c8y.Hardware;

public class SimulatedLightSensor extends AbstractLightSensor {

    private double illuminance = 50;

    public SimulatedLightSensor(String id) {
        super(id);
    }

    @Override
    Hardware getHardware() {
        return new Hardware(
                "Simulated Light Sensor",
                "098245687332343",
                "1.0.0"
        );
    }

    protected double getIlluminance() {
        // simulate gradual illuminance change
        double maxStep = 10.0;
        double randomChange = Math.random() * maxStep - maxStep / 2.0;

        illuminance = Math.min(Math.max(illuminance + randomChange, 0.0), 100.0);

        return illuminance;
    }
}
