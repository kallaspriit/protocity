package com.stagnationlab.c8y.driver.devices;

import c8y.LightMeasurement;
import com.stagnationlab.c8y.driver.fragments.LightSensor;

import java.math.BigDecimal;

public abstract class AbstractLightSensor extends AbstractDevice {

    protected AbstractLightSensor(String id) {
        super(id);
    }

    @Override
    protected String getType() {
        return "Light";
    }

    @Override
    protected Object getSensorFragment() {
        return new LightSensor();
    }

    protected void reportIlluminance(float illuminance) {
        LightMeasurement lightMeasurement = new LightMeasurement();
        lightMeasurement.setIlluminance(new BigDecimal(illuminance));

        reportMeasurement(lightMeasurement);
    }

}
