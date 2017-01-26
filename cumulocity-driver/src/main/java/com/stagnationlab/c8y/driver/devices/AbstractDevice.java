package com.stagnationlab.c8y.driver.devices;

import static java.util.concurrent.TimeUnit.MILLISECONDS;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.cumulocity.model.idtype.GId;
import com.cumulocity.rest.representation.event.EventRepresentation;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.rest.representation.measurement.MeasurementRepresentation;
import com.cumulocity.sdk.client.Platform;
import com.cumulocity.sdk.client.event.EventApi;
import com.cumulocity.sdk.client.inventory.InventoryApi;
import com.cumulocity.sdk.client.measurement.MeasurementApi;
import com.stagnationlab.c8y.driver.services.DeviceManager;
import com.stagnationlab.c8y.driver.services.Util;

import c8y.Hardware;
import c8y.lx.driver.Driver;
import c8y.lx.driver.OperationExecutor;

/**
 * Base class for devices.
 *
 * Life cycle order:
 * 1. initialize()
 * 2. initialize(Platform platform)
 * 3. initializeInventory(ManagedObjectRepresentation self)
 * 4. getSupportedOperations()
 * 5. start()
 */
@SuppressWarnings("WeakerAccess")
public abstract class AbstractDevice implements Driver {

    private static final Logger log = LoggerFactory.getLogger(AbstractDevice.class);

    protected Platform platform;
    protected MeasurementApi measurementApi;
    protected EventApi eventApi;
    protected InventoryApi inventoryApi;
    protected ManagedObjectRepresentation device;
    protected GId parentId;
    protected GId myId;
    protected final List<OperationExecutor> operationExecutors;
    protected final List<Driver> children = new ArrayList<>();
    protected final String id;

    protected AbstractDevice(String id) {
        this.id = id;

        operationExecutors = new ArrayList<>();
    }

    public String getId() {
    	return id;
    }

    protected abstract String getType();

    protected Object getSensorFragment() {
        return null;
    }

    protected Hardware getHardware() {
        return null;
    }

	protected void createChildren() {}

    @Override
    public void initialize() throws Exception {
    	createChildren();

	    log.debug("initializing {} with {} {}", id, children.size(), children.size() == 1 ? "child" : "children");

	    for (Driver child : children) {
	    	child.initialize();
	    }
    }

    @Override
    public void initialize(Platform platform) throws Exception {
	    log.debug("initializing {} with platform", id);

        this.platform = platform;
        this.measurementApi = platform.getMeasurementApi();
        this.eventApi = platform.getEventApi();
        this.inventoryApi = platform.getInventoryApi();

	    for (Driver child : children) {
		    child.initialize(platform);
	    }
    }

	@Override
	public void initializeInventory(ManagedObjectRepresentation owner) {
		log.debug("initializing {} inventory, owner: {}", id, owner.getId() == null ? "none" : owner.getId().getValue());

		debugManagedObject(owner);

		for (Driver child : children) {
			child.initializeInventory(owner);
		}
	}

	@Override
	public void discoverChildren(ManagedObjectRepresentation parent) {
		parentId = parent.getId();

		log.debug("discovering {} children for parent with id {}", id, parentId.getValue());
		debugManagedObject(parent);

		device = DeviceManager.create(
				id,
				getType(),
				platform,
				parent,
				getHardware(),
				getSupportedOperations(),
				getSensorFragment()
		);

		myId = device.getId();

		log.debug("created device for {} with id: {}", id, myId.getValue());
		debugManagedObject(device);

		for (Driver child : children) {
			child.discoverChildren(device);
		}
	}

	@Override
	public void start() {
		log.debug("starting {}", id);

		for (Driver child : children) {
			child.start();
		}
	}

    @Override
    public OperationExecutor[] getSupportedOperations() {
	    log.debug("list of supported operations for {} requested ({} total):", id, operationExecutors.size());

	    for (OperationExecutor operationExecutor : operationExecutors) {
	    	log.debug("  - {}", operationExecutor.supportedOperationType());
	    }

        return operationExecutors.toArray(new OperationExecutor[operationExecutors.size()]);
    }

    protected void registerChild(Driver child) {
    	if (child instanceof AbstractDevice) {
    		AbstractDevice device = (AbstractDevice)child;

		    log.debug("registering {} child: {} ({})", id, device.getId(), child.getClass().getSimpleName());
	    } else {
		    log.debug("registering {} child: {}", id, child.getClass().getSimpleName());
	    }

    	children.add(child);
    }

    protected void registerOperationExecutor(OperationExecutor operationExecutor) {
        log.info("registering operation executor for {} of type: {}", id, operationExecutor.supportedOperationType());

        operationExecutors.add(operationExecutor);
    }

    protected MeasurementRepresentation reportMeasurement(Object measurement, String type) {
        log.debug("reporting measurement for {} of type {}: {}", id, type, Util.stringify(measurement));

        MeasurementRepresentation measurementRepresentation = new MeasurementRepresentation();

        measurementRepresentation.setSource(device);
        measurementRepresentation.setType(type);
        measurementRepresentation.set(measurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);

        return measurementRepresentation;
    }

	protected MeasurementRepresentation reportMeasurement(Object measurement) {
		return reportMeasurement(measurement, measurement.getClass().getSimpleName());
	}

    protected void reportEvent(EventRepresentation eventRepresentation) {
        log.debug("reporting event for {} of type {}: {}", id, eventRepresentation.getClass().getSimpleName(), Util.stringify(eventRepresentation));

        eventRepresentation.setSource(device);

        eventApi.create(eventRepresentation);
    }

    protected ManagedObjectRepresentation updateState(Object... properties) {
        log.debug("updating state of {}: {}", id, Util.stringify(properties));

        ManagedObjectRepresentation managedObjectRepresentation = new ManagedObjectRepresentation();
        managedObjectRepresentation.setId(device.getId());

        for (Object property : properties) {
            managedObjectRepresentation.set(property);
        }

        device = platform.getInventoryApi().update(managedObjectRepresentation);

        return managedObjectRepresentation;
    }

    @SuppressWarnings("SameParameterValue")
    protected ScheduledFuture<?> setInterval(Runnable runnable, long intervalMs) {
        log.info("creating an interval for {} every {}ms", id, intervalMs);

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

	protected void debugManagedObject(ManagedObjectRepresentation o) {
		log.debug("  > id: {}, type: {}, name: {}", o.getId() == null ? "n/a" : o.getId().getValue(), o.getType(), o.getName());
	}
}
