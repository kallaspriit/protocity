package com.stagnationlab.c8y.driver.devices;

import com.stagnationlab.c8y.driver.fragments.LightSensor;
import com.stagnationlab.c8y.driver.measurements.LightMeasurement;

public abstract class AbstractLightSensor extends AbstractDevice {

    private final LightSensor lightSensor = new LightSensor();

    protected AbstractLightSensor(String id) {
        super(id);
    }

    @Override
    protected String getType() {
        return lightSensor.getClass().getSimpleName();
    }

    @Override
    protected Object getSensorFragment() {
        return lightSensor;
    }

    protected void reportIlluminance(float illuminance) {
        LightMeasurement lightMeasurement = new LightMeasurement(illuminance);

        reportMeasurement(lightMeasurement);
    }

}
