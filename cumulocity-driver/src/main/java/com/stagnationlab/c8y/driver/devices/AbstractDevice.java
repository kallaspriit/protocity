package com.stagnationlab.c8y.driver.devices;

import static java.util.concurrent.TimeUnit.MILLISECONDS;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;

import lombok.extern.slf4j.Slf4j;

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
@Slf4j
public abstract class AbstractDevice implements Driver {

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
    protected boolean isApiReady = false;

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

    @SuppressWarnings("SameReturnValue")
    protected Hardware getHardware() {
        return null;
    }

	protected void setup() throws Exception {}

    @Override
    public void initialize() throws Exception {
	    log.debug("initializing '{}' with {} {}", id, children.size(), children.size() == 1 ? "child" : "children");

	    try {
		    setup();
	    } catch (Exception e) {
		    log.warn("setup of device '{}' failed ({} - {})", id, e.getClass().getSimpleName(), e.getMessage(), e);

		    return;
	    }

	    for (Driver child : children) {
	    	try {
			    child.initialize();
		    } catch (Exception e) {
			    debugDriverException(child, e, "initializing failed");

			    throw e;
		    }
	    }
    }

    @Override
    public void initialize(Platform platform) throws Exception {
	    log.debug("initializing '{}' with platform", id);

        this.platform = platform;
        this.measurementApi = platform.getMeasurementApi();
        this.eventApi = platform.getEventApi();
        this.inventoryApi = platform.getInventoryApi();
        this.isApiReady = true;

	    for (Driver child : children) {
	    	try {
			    child.initialize(platform);
		    } catch (Exception e) {
				debugDriverException(child, e, "initializing platform connectivity failed");

				throw e;
		    }
	    }
    }

	@Override
	public void initializeInventory(ManagedObjectRepresentation owner) {
		for (Driver child : children) {
			try {
				child.initializeInventory(owner);
			} catch (Exception e) {
				debugDriverException(child, e, "initializing inventory failed");

				throw e;
			}
		}

		log.debug("initializing '{}' inventory, owner: {}", id, owner.getId() == null ? "none" : owner.getId().getValue());
		debugManagedObject(owner);
	}

	@Override
	public void discoverChildren(ManagedObjectRepresentation parent) {
		parentId = parent.getId();

		log.debug("discovering '{}' children for parent with id {}", id, parentId.getValue());
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

		log.debug("created device for '{}' with id: {}", id, myId.getValue());
		debugManagedObject(device);

		for (Driver child : children) {
			try {
				child.discoverChildren(device);
			} catch (Exception e) {
				debugDriverException(child, e, "discovering children failed");

				throw e;
			}
		}
	}

	@Override
	public void start() {
		for (Driver child : children) {
			try {
				child.start();
			} catch (Exception e) {
				debugDriverException(child, e, "initializing platform connectivity failed");

				throw e;
			}
		}

		log.debug("starting '{}'", id);
	}

    @Override
    public OperationExecutor[] getSupportedOperations() {
	    List<OperationExecutor> combinedOperationExecutors = new ArrayList<>();

	    for (OperationExecutor myOperationExecutor : operationExecutors) {
	    	combinedOperationExecutors.add(myOperationExecutor);
	    }

	    for (Driver child : children) {
		    List<OperationExecutor> childOperationExecutors = Arrays.asList(child.getSupportedOperations());

		    for (OperationExecutor childOperationExecutor : childOperationExecutors) {
		    	combinedOperationExecutors.add(childOperationExecutor);
		    }
	    }

	    log.trace("returning list of supported operations for '{}' ({} total):", id, operationExecutors.size());

	    return combinedOperationExecutors.toArray(new OperationExecutor[combinedOperationExecutors.size()]);
    }

    public void shutdown() {
	    log.debug("shutting down '{}'", id);

	    for (Driver child : children) {
	    	if (!(child instanceof AbstractDevice)) {
	    		continue;
		    }

	    	AbstractDevice device = (AbstractDevice)child;

		    try {
			    device.shutdown();
		    } catch (Exception e) {
			    debugDriverException(child, e, "shutting down device failed");

			    throw e;
		    }
	    }
    }

    protected void registerChild(Driver child) {
    	if (child instanceof AbstractDevice) {
    		AbstractDevice device = (AbstractDevice)child;

		    log.debug("registering '{}' child: {} ({})", id, device.getId(), child.getClass().getSimpleName());
	    } else {
		    log.debug("registering '{}' child: {}", id, child.getClass().getSimpleName());
	    }

    	children.add(child);
    }

    protected void registerOperationExecutor(OperationExecutor operationExecutor) {
        log.debug("registering operation executor for '{}' of type: {}", id, operationExecutor.supportedOperationType());

        operationExecutors.add(operationExecutor);
    }

    protected MeasurementRepresentation reportMeasurement(Object measurement, String type) {
        log.trace("reporting measurement for '{}' of type {}: {}", id, type, Util.stringify(measurement));

        MeasurementRepresentation measurementRepresentation = new MeasurementRepresentation();

        measurementRepresentation.setSource(device);
        measurementRepresentation.setType(type);
        measurementRepresentation.set(measurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);

        return measurementRepresentation;
    }

	@SuppressWarnings("UnusedReturnValue")
	protected MeasurementRepresentation reportMeasurement(Object measurement) {
		return reportMeasurement(measurement, measurement.getClass().getSimpleName());
	}

    protected void reportEvent(EventRepresentation eventRepresentation) {
        log.trace("reporting event for '{}' of type {}: {}", id, eventRepresentation.getClass().getSimpleName(), Util.stringify(eventRepresentation));

        eventRepresentation.setSource(device);

        eventApi.create(eventRepresentation);
    }

    @SuppressWarnings("UnusedReturnValue")
    protected ManagedObjectRepresentation updateState(Object... properties) {
        ManagedObjectRepresentation managedObjectRepresentation = new ManagedObjectRepresentation();

        for (Object property : properties) {
            managedObjectRepresentation.set(property);
        }

	    if (device == null || inventoryApi == null) {
		    log.warn("updating state of '{}' requested but initialize with platform is not yet called, APIs not available yet", id);
	    } else {
		    log.trace("updating state of '{}': {}", id, Util.stringify(properties));

		    managedObjectRepresentation.setId(device.getId());
		    device = inventoryApi.update(managedObjectRepresentation);
	    }

        return managedObjectRepresentation;
    }

    @SuppressWarnings({ "SameParameterValue", "UnusedReturnValue" })
    protected ScheduledFuture<?> setInterval(Runnable runnable, long intervalMs) {
        log.info("creating an interval for '{}' every {}ms", id, intervalMs);

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

	protected void debugDriverException(Driver child, Exception e, String message) {
		if (child instanceof AbstractDevice) {
			AbstractDevice childDevice = (AbstractDevice)child;

			log.warn("{}:{} - {} ({} - {})", childDevice.getClass().getSimpleName(), childDevice.getId(), message, e.getClass().getSimpleName(), e.getMessage(), e);
		} else {
			log.warn("{} - {} ({} - {})", child.getClass().getSimpleName(), message, e.getClass().getSimpleName(), e.getMessage(), e);
		}

		e.printStackTrace();
	}
}
