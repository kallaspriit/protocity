package com.stagnationlab.c8y.driver.devices.simulated;

import com.stagnationlab.c8y.driver.devices.AbstractLightSensor;

public class SimulatedLightSensor extends AbstractLightSensor {

    private float illuminance = 50.0f;

    public SimulatedLightSensor(String id) {
        super(id);
    }

    @Override
    public void start() {
        super.start();

        setInterval(() -> reportIlluminance(getSimulatedIlluminance()), 5000);
    }

    private float getSimulatedIlluminance() {
        float maxStep = 10.0f;
        float randomChange = (float)Math.random() * maxStep - maxStep / 2.0f;

        illuminance = Math.min(Math.max(illuminance + randomChange, 0.0f), 100.0f);

        return illuminance;
    }

}
