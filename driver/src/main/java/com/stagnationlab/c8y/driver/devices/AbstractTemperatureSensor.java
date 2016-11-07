package com.stagnationlab.c8y.driver.devices;

import c8y.Hardware;
import c8y.TemperatureMeasurement;
import c8y.TemperatureSensor;
import c8y.lx.driver.MeasurementPollingDriver;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.stagnationlab.c8y.driver.services.DeviceManager;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.math.BigDecimal;

public abstract class AbstractTemperatureSensor extends MeasurementPollingDriver {

    private static final Logger log = LoggerFactory.getLogger(AbstractTemperatureSensor.class);

    private static final String TYPE = "Temperature";

    private final String id;

    protected AbstractTemperatureSensor(String id) {
        super("c8y_" + TYPE + "Sensor", "c8y." + TYPE.toLowerCase(), 5000);

        this.id = id;
    }

    @Override
    public void initialize() throws Exception {
        log.info("initializing");
    }

    @Override
    public void discoverChildren(ManagedObjectRepresentation parent) {
        log.info("creating child");

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
