package com.stagnationlab.c8y.driver.devices;

import c8y.Relay;
import c8y.lx.driver.OperationExecutor;
import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.stagnationlab.c8y.driver.measurements.RelayStateMeasurement;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public abstract class AbstractRelayActuator extends AbstractDevice {

    private static final Logger log = LoggerFactory.getLogger(AbstractRelayActuator.class);

    private final Relay relay = new Relay();

    protected AbstractRelayActuator(String id) {
        super(id);
    }

    @Override
    protected String getType() {
        return "Relay";
    }

    @Override
    protected Object getSensorFragment() {
        return relay;
    }

    @Override
    public void initialize() throws Exception {
        registerOperationExecutor(new OperationExecutor() {
            @Override
            public String supportedOperationType() {
                return "c8y_Relay";
            }

            @Override
            public void execute(OperationRepresentation operation, boolean cleanup) throws Exception {
                if (!childDevice.getId().equals(operation.getDeviceId())) {
                    log.info("ignore relay operation because of id mismatch ({} vs {})", operation.getDeviceId(), childDevice.getId());

                    return;
                }

                Relay relay = operation.get(Relay.class);
                Relay.RelayState relayState = relay.getRelayState();

                if (cleanup) {
                    log.info("got relay cleanup operation with state '{}', ignoring it and reporting failed status", relayState);

                    operation.setStatus(OperationStatus.FAILED.toString());

                    return;
                }

                boolean shouldRelayBeClosed = relayState == Relay.RelayState.CLOSED;

                setRelayClosed(shouldRelayBeClosed);

                operation.setStatus(OperationStatus.SUCCESSFUL.toString());
            }
        });
    }

    @Override
    public void start() {
        log.info("starting '{}', relay is currently {}", id, relay.getRelayState() == Relay.RelayState.CLOSED ? "closed" : "open");
    }

    protected abstract void applyRelayState(boolean isRelayClosed);

    private void setRelayClosed(boolean isRelayClosed, boolean isForced) {
        boolean isRelayCurrentlyOn = relay.getRelayState() == Relay.RelayState.CLOSED;

        if (!isForced && isRelayClosed == isRelayCurrentlyOn) {
            log.info("'{}' is already {}, ignoring request", id, isRelayClosed ? "closed" : "open");

            return;
        }

        relay.setRelayState(isRelayClosed ? Relay.RelayState.CLOSED : Relay.RelayState.OPEN);

        applyRelayState(isRelayClosed);
        updateState(relay);
        sendStateMeasurement();

        log.info("'{}' is now {}", id, isRelayClosed ? "closed" : "open");
    }

    private void setRelayClosed(boolean isRelayClosed) {
        setRelayClosed(isRelayClosed, false);
    }

    private void sendStateMeasurement() {
        RelayStateMeasurement measurement = new RelayStateMeasurement();

        // send inverse measurement first to get a square graph
        measurement.setState(relay.getRelayState() == Relay.RelayState.CLOSED ? Relay.RelayState.OPEN : Relay.RelayState.CLOSED);
        reportMeasurement(measurement);

        // send current state
        measurement.setState(relay.getRelayState());
        reportMeasurement(measurement);
    }
}
