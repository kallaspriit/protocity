package com.stagnationlab.c8y.driver.devices;

import c8y.Hardware;
import c8y.lx.driver.Driver;
import c8y.lx.driver.OperationExecutor;
import com.cumulocity.rest.representation.event.EventRepresentation;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.rest.representation.measurement.MeasurementRepresentation;
import com.cumulocity.sdk.client.Platform;
import com.cumulocity.sdk.client.event.EventApi;
import com.cumulocity.sdk.client.inventory.InventoryApi;
import com.cumulocity.sdk.client.measurement.MeasurementApi;
import com.stagnationlab.c8y.driver.services.DeviceManager;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;

import static java.util.concurrent.TimeUnit.MILLISECONDS;

@SuppressWarnings("WeakerAccess")
public abstract class AbstractDevice implements Driver {

    protected Platform platform;
    protected MeasurementApi measurementApi;
    protected EventApi eventApi;
    protected InventoryApi inventoryApi;
    protected ManagedObjectRepresentation childDevice;
    protected final List<OperationExecutor> operationExecutors;
    protected final String id;

    protected AbstractDevice(String id) {
        this.id = id;

        operationExecutors = new ArrayList<>();
    }

    @Override
    public void initializeInventory(ManagedObjectRepresentation managedObjectRepresentation) {

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
        this.eventApi = platform.getEventApi();
        this.inventoryApi = platform.getInventoryApi();
    }

    @Override
    public OperationExecutor[] getSupportedOperations() {
        return operationExecutors.toArray(new OperationExecutor[operationExecutors.size()]);
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

    protected void registerOperationExecutor(OperationExecutor operationExecutor) {
        operationExecutors.add(operationExecutor);
    }

    protected MeasurementRepresentation reportMeasurement(Object measurement, String type) {
        MeasurementRepresentation measurementRepresentation = new MeasurementRepresentation();

        measurementRepresentation.setSource(childDevice);
        measurementRepresentation.setType(type);
        measurementRepresentation.set(measurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);

        return measurementRepresentation;
    }

    protected MeasurementRepresentation reportMeasurement(Object measurement) {
        return reportMeasurement(measurement, getType());
    }

    protected void reportEvent(EventRepresentation eventRepresentation) {
        eventRepresentation.setSource(childDevice);

        eventApi.create(eventRepresentation);
    }

    protected ManagedObjectRepresentation updateState(Object... properties) {
        ManagedObjectRepresentation managedObjectRepresentation = new ManagedObjectRepresentation();
        managedObjectRepresentation.setId(childDevice.getId());

        for (Object property : properties) {
            managedObjectRepresentation.set(property);
        }

        childDevice = platform.getInventoryApi().update(managedObjectRepresentation);

        return managedObjectRepresentation;
    }

    protected ScheduledFuture<?> setInterval(Runnable runnable, long intervalMs) {
        ScheduledExecutorService executorService = Executors.newSingleThreadScheduledExecutor(
                r -> new Thread(r, getType() + "Interval")
        );

        long now = new Date().getTime();
        long initialDelay = intervalMs - (now % intervalMs);

        return executorService.scheduleAtFixedRate(
                runnable,
                initialDelay,
                intervalMs,
                MILLISECONDS
        );
    }
}
