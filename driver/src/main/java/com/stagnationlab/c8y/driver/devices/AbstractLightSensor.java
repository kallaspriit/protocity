package com.stagnationlab.c8y.driver.devices;

import c8y.LightMeasurement;
import com.stagnationlab.c8y.driver.fragments.LightSensor;

import java.math.BigDecimal;

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
        LightMeasurement lightMeasurement = new LightMeasurement();
        lightMeasurement.setIlluminance(new BigDecimal(illuminance));

        reportMeasurement(lightMeasurement);
    }

}
