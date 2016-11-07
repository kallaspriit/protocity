package com.stagnationlab.c8y.driver.sensors;

import c8y.Hardware;
import c8y.lx.driver.Driver;
import c8y.lx.driver.OperationExecutor;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.rest.representation.measurement.MeasurementRepresentation;
import com.cumulocity.sdk.client.Platform;
import com.cumulocity.sdk.client.event.EventApi;
import com.cumulocity.sdk.client.measurement.MeasurementApi;
import com.stagnationlab.c8y.driver.DeviceManager;
import com.stagnationlab.c8y.driver.events.ButtonPressedEvent;
import com.stagnationlab.c8y.driver.events.ButtonReleasedEvent;
import com.stagnationlab.c8y.driver.measurements.ButtonStateMeasurement;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Date;

public abstract class AbstractButtonSensor implements Driver {

    class ButtonSensor {}

    public enum State {
        BUTTON_RELEASED,
        BUTTON_PRESSED
    }

    private static final Logger log = LoggerFactory.getLogger(AbstractButtonSensor.class);

    private static final String TYPE = "Button";

    private Platform platform;
    private EventApi eventApi;
    private MeasurementApi measurementApi;
    private ManagedObjectRepresentation childDevice;
    private final String id;

    AbstractButtonSensor(String id) {
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
                new ButtonSensor()
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

    void triggerButtonPressed() {
        ButtonPressedEvent event = new ButtonPressedEvent();
        event.setSource(childDevice);

        eventApi.create(event);

        sendButtonMeasurement(true);
    }

    void triggerButtonReleased() {
        ButtonReleasedEvent event = new ButtonReleasedEvent();
        event.setSource(childDevice);

        eventApi.create(event);

        sendButtonMeasurement(false);
    }

    private void sendButtonMeasurement(boolean isButtonPressed) {
        MeasurementRepresentation measurementRepresentation = new MeasurementRepresentation();

        measurementRepresentation.setSource(childDevice);
        measurementRepresentation.setType("MotionDetected");

        // send inverse measurement first to get a square graph
        ButtonStateMeasurement measurement = new ButtonStateMeasurement();
        measurement.setState(isButtonPressed ? State.BUTTON_RELEASED : State.BUTTON_PRESSED);
        measurementRepresentation.set(measurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);

        // send the current state
        measurement = new ButtonStateMeasurement();
        measurement.setState(isButtonPressed ? State.BUTTON_PRESSED : State.BUTTON_RELEASED);
        measurementRepresentation.set(measurement);
        measurementRepresentation.setTime(new Date());

        measurementApi.create(measurementRepresentation);
    }

}
