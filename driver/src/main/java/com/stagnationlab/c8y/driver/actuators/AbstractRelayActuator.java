package com.stagnationlab.c8y.driver.actuators;

import c8y.Hardware;
import c8y.Relay;
import c8y.lx.driver.Driver;
import c8y.lx.driver.OperationExecutor;
import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.rest.representation.measurement.MeasurementRepresentation;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.cumulocity.sdk.client.Platform;
import com.cumulocity.sdk.client.measurement.MeasurementApi;
import com.stagnationlab.c8y.driver.DeviceManager;
import com.stagnationlab.c8y.driver.measurements.RelayStateMeasurement;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Date;

abstract class AbstractRelayActuator implements Driver, OperationExecutor {

    private static final Logger log = LoggerFactory.getLogger(AbstractRelayActuator.class);

    private static final String TYPE = "Relay";

    private final Relay relay = new Relay();
    private Platform platform;
    private MeasurementApi measurementApi;
    private ManagedObjectRepresentation childDevice;
    private final String id;

    AbstractRelayActuator(String id) {
        this.id = id;

    }

    @Override
    public void initialize() throws Exception {
        log.info("initializing");
    }

    @Override
    public void initialize(Platform platform) throws Exception {
        log.info("initializing platform");

        this.platform = platform;

        measurementApi = platform.getMeasurementApi();
    }

    @Override
    public void discoverChildren(ManagedObjectRepresentation parent) {
        log.info("creating childDevice");

        childDevice = DeviceManager.createChild(
                id,
                TYPE,
                platform,
                parent,
                getHardware(),
                getSupportedOperations(),
                new Relay()
        );

        log.info("created managed object: " + childDevice.getId());
    }

    @Override
    public String supportedOperationType() {
        return "c8y_" + TYPE;
    }

    @Override
    public OperationExecutor[] getSupportedOperations() {
        return new OperationExecutor[]{this};
    }

    @Override
    public void initializeInventory(ManagedObjectRepresentation managedObject) {
        log.info("initialize inventory");
    }

    @Override
    public void start() {
        setRelayOn(false, true);
    }

    @Override
    public void execute(OperationRepresentation operation, boolean cleanup) throws Exception {
        if (!childDevice.getId().equals(operation.getDeviceId())) {
            return;
        }

        log.info("performing " + (cleanup ? "cleanup" : "normal") + " execution for device '" + operation.getDeviceId() + "'");

        if (cleanup) {
            operation.setStatus(OperationStatus.FAILED.toString());

            return;
        } else {
            operation.setStatus(OperationStatus.SUCCESSFUL.toString());
        }

        Relay relay = operation.get(Relay.class);
        Relay.RelayState relayState = relay.getRelayState();

        boolean shouldRelayBeOn = relayState == Relay.RelayState.CLOSED;

        setRelayOn(shouldRelayBeOn);
    }

    abstract Hardware getHardware();

    abstract void applyRelayState(boolean isRelayOn);

    private void setRelayOn(boolean isRelayOn, boolean isForced) {
        boolean isRelayCurrentlyOn = relay.getRelayState() == Relay.RelayState.CLOSED;

        if (!isForced && isRelayOn == isRelayCurrentlyOn) {
            log.info("relay is already " + (isRelayOn ? "on" : "off") + ", ignoring request");

            return;
        }

        applyRelayState(isRelayOn);
        updateManagedObjectState(isRelayOn);
        sendStateMeasurement(isRelayOn);
    }

    private void setRelayOn(boolean isRelayOn) {
        log.info("turning relay '" + id + "' " + (isRelayOn ? "on" : "off"));

        setRelayOn(isRelayOn, false);
    }

    private void updateManagedObjectState(boolean isRelayOn) {
        log.info("updating relay managed object state to " + (isRelayOn ? "on" : "off") + " state");

        relay.setRelayState(isRelayOn ? Relay.RelayState.CLOSED : Relay.RelayState.OPEN);

        ManagedObjectRepresentation updateRelayManagedObject = new ManagedObjectRepresentation();
        updateRelayManagedObject.setId(childDevice.getId());
        updateRelayManagedObject.set(relay);

        childDevice = platform.getInventoryApi().update(updateRelayManagedObject);
    }

    private void sendStateMeasurement(boolean isRelayOn) {
        log.info("sending relay state change measurement: " + (isRelayOn ? "on" : "off") + " state");

        MeasurementRepresentation measurementRepresentation = new MeasurementRepresentation();

        measurementRepresentation.setSource(childDevice);
        measurementRepresentation.setType("c8y_" + TYPE);

        // send inverse measurement first to get a square graph
        RelayStateMeasurement relayStateInverseMeasurement = new RelayStateMeasurement();
        relayStateInverseMeasurement.setState(relay.getRelayState() == Relay.RelayState.CLOSED ? Relay.RelayState.OPEN : Relay.RelayState.CLOSED);
        measurementRepresentation.set(relayStateInverseMeasurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);

        // send the current state
        RelayStateMeasurement relayStateMeasurement = new RelayStateMeasurement();
        relayStateMeasurement.setState(relay.getRelayState());
        measurementRepresentation.set(relayStateMeasurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);
    }
}
