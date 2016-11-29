package com.stagnationlab.c8y.driver.devices;

import java.math.BigDecimal;

import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.stagnationlab.c8y.driver.services.DeviceManager;

import c8y.Hardware;
import c8y.TemperatureMeasurement;
import c8y.TemperatureSensor;
import c8y.lx.driver.MeasurementPollingDriver;

// TODO Convert to use AbstractDevice
public abstract class AbstractTemperatureSensor extends MeasurementPollingDriver {

    private static final String TYPE = "TemperatureSensor";

    private final String id;

    protected AbstractTemperatureSensor(String id) {
        super("c8y_" + TYPE + "Sensor", "c8y." + TYPE.toLowerCase(), 5000);

        this.id = id;
    }

    @Override
    public void discoverChildren(ManagedObjectRepresentation parent) {
        ManagedObjectRepresentation childDevice = DeviceManager.createChild(
                id,
                TYPE,
                getPlatform(),
                parent,
                getHardware(),
                getSupportedOperations(),
                new TemperatureSensor()
        );

        setSource(childDevice);
    }

    @Override
    public void run() {
        double temperature = getTemperature();

        TemperatureMeasurement temperatureMeasurement = new TemperatureMeasurement();
        temperatureMeasurement.setTemperature(new BigDecimal(temperature));

        sendMeasurement(temperatureMeasurement);

        //log.info("sending temperature measurement: " + temperature);
    }

    protected abstract Hardware getHardware();

    protected abstract double getTemperature();

}
