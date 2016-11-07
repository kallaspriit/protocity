package com.stagnationlab.c8y.driver.sensors;

import c8y.Hardware;
import c8y.MotionSensor;
import c8y.lx.driver.Driver;
import c8y.lx.driver.OperationExecutor;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.rest.representation.measurement.MeasurementRepresentation;
import com.cumulocity.sdk.client.Platform;
import com.cumulocity.sdk.client.event.EventApi;
import com.cumulocity.sdk.client.measurement.MeasurementApi;
import com.stagnationlab.c8y.driver.DeviceManager;
import com.stagnationlab.c8y.driver.events.MotionDetectedEvent;
import com.stagnationlab.c8y.driver.events.MotionEndedEvent;
import com.stagnationlab.c8y.driver.measurements.MotionStateMeasurement;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Date;

abstract public class AbstractMotionSensor implements Driver {

    public enum State {
        MOTION_ENDED,
        MOTION_DETECTED
    }

    private static final Logger log = LoggerFactory.getLogger(AbstractMotionSensor.class);

    private static final String TYPE = "Motion";

    private Platform platform;
    private EventApi eventApi;
    private MeasurementApi measurementApi;
    private ManagedObjectRepresentation childDevice;
    private final String id;

    AbstractMotionSensor(String id) {
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

        eventApi = platform.getEventApi();
        measurementApi = platform.getMeasurementApi();
    }

    @Override
    public void discoverChildren(ManagedObjectRepresentation parent) {
        log.info("creating child");

        childDevice = DeviceManager.createChild(
                id,
                TYPE,
                platform,
                parent,
                getHardware(),
                getSupportedOperations(),
                new MotionSensor()
        );
    }

    @Override
    public OperationExecutor[] getSupportedOperations() {
        return new OperationExecutor[0];
    }

    @Override
    public void initializeInventory(ManagedObjectRepresentation parent) {
        log.info("initializing inventory");
    }

    @Override
    public void start() {
        log.info("starting driver");
    }

    abstract Hardware getHardware();

    void triggerMotionDetected() {
        MotionDetectedEvent event = new MotionDetectedEvent();
        event.setSource(childDevice);

        eventApi.create(event);

        sendMotionMeasurement(true);
    }

    void triggerMotionEnded() {
        MotionEndedEvent event = new MotionEndedEvent();
        event.setSource(childDevice);

        eventApi.create(event);

        sendMotionMeasurement(false);
    }

    private void sendMotionMeasurement(boolean isMotionDetected) {
        MeasurementRepresentation measurementRepresentation = new MeasurementRepresentation();

        measurementRepresentation.setSource(childDevice);
        measurementRepresentation.setType("c8y");

        // send inverse measurement first to get a square graph
        MotionStateMeasurement measurement = new MotionStateMeasurement();
        measurement.setState(isMotionDetected ? State.MOTION_ENDED : State.MOTION_DETECTED);
        measurementRepresentation.set(measurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);

        // send the current state
        measurement = new MotionStateMeasurement();
        measurement.setState(isMotionDetected ? State.MOTION_DETECTED : State.MOTION_ENDED);
        measurementRepresentation.set(measurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);
    }
}
