package com.stagnationlab.c8y.driver.devices;

import c8y.LightMeasurement;
import c8y.LightSensor;
import com.cumulocity.rest.representation.measurement.MeasurementRepresentation;

import java.math.BigDecimal;
import java.util.Date;

public abstract class AbstractLightSensor extends AbstractMeasurementSensor {

    protected AbstractLightSensor(String id) {
        super(id);
    }

    protected String getType() {
        return "Light";
    }

    protected Object getSensorFragment() {
        return new LightSensor();
    }

    protected void reportIlluminance(float illuminance) {
        LightMeasurement lightMeasurement = new LightMeasurement();
        lightMeasurement.setIlluminance(new BigDecimal(illuminance));

        MeasurementRepresentation measurementRepresentation = new MeasurementRepresentation();

        measurementRepresentation.setSource(childDevice);
        measurementRepresentation.setType(getType());
        measurementRepresentation.set(lightMeasurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);
    }

}
