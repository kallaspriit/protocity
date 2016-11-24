package com.stagnationlab.c8y.driver.devices;

import c8y.Hardware;
import c8y.LightSensor;
import c8y.lx.driver.Driver;
import c8y.lx.driver.OperationExecutor;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.sdk.client.Platform;
import com.cumulocity.sdk.client.measurement.MeasurementApi;
import com.stagnationlab.c8y.driver.services.DeviceManager;

import java.util.Date;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;

import static java.util.concurrent.TimeUnit.MILLISECONDS;

@SuppressWarnings("WeakerAccess")
public abstract class AbstractMeasurementSensor implements Driver {

    protected Platform platform;
    protected MeasurementApi measurementApi;
    protected ManagedObjectRepresentation childDevice;
    protected final String id;

    protected AbstractMeasurementSensor(String id) {
        this.id = id;
    }

    @Override
    public void initializeInventory(ManagedObjectRepresentation mo) {

    }

    @Override
    public void start() {

    }

    @Override
    public void initialize() throws Exception {

    }

    @Override
    public void initialize(Platform platform) throws Exception {
        this.platform = platform;
        this.measurementApi = platform.getMeasurementApi();
    }

    @Override
    public OperationExecutor[] getSupportedOperations() {
        return new OperationExecutor[0];
    }

    @Override
    public void discoverChildren(ManagedObjectRepresentation parent) {
        childDevice = DeviceManager.createChild(
                id,
                getType(),
                platform,
                parent,
                getHardware(),
                getSupportedOperations(),
                getSensorFragment()
        );
    }

    protected abstract String getType();

    protected Hardware getHardware() {
        return null;
    }

    protected Object getSensorFragment() {
        return null;
    }

    protected ScheduledFuture<?> setInterval(Runnable runnable, long intervalMs) {
        ScheduledExecutorService executorService = Executors.newSingleThreadScheduledExecutor(
                r -> new Thread(r, getType() + "Interval")
        );

        long now = new Date().getTime();
        long initialDelay = intervalMs - (now % intervalMs);

        return executorService.scheduleAtFixedRate(
                () -> {

                },
                initialDelay,
                intervalMs,
                MILLISECONDS
        );
    }
}
