package com.stagnationlab.c8y.driver.devices;

import com.stagnationlab.c8y.driver.events.ButtonPressedEvent;
import com.stagnationlab.c8y.driver.fragments.ButtonSensor;
import com.stagnationlab.c8y.driver.measurements.ButtonStateMeasurement;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public abstract class AbstractButtonSensor extends AbstractDevice {

    private static final Logger log = LoggerFactory.getLogger(AbstractButtonSensor.class);

    private final ButtonSensor buttonSensor = new ButtonSensor();

    AbstractButtonSensor(String id) {
        super(id);
    }

    @Override
    protected String getType() {
        return "Button";
    }

    @Override
    protected Object getSensorFragment() {
        return buttonSensor;
    }

    protected void setButtonPressed(boolean isButtonPressed) {
        log.info("button {} is now {}", id, isButtonPressed ? "pressed" : "released");

        buttonSensor.setButtonState(isButtonPressed ? ButtonSensor.ButtonState.PRESSED : ButtonSensor.ButtonState.RELEASED);

        reportEvent(new ButtonPressedEvent());
        updateState(buttonSensor);
        sendStateMeasurement();
    }

    private void sendStateMeasurement() {
        ButtonStateMeasurement measurement = new ButtonStateMeasurement();

        // send inverse measurement first to get a square graph
        measurement.setState(buttonSensor.getButtonState() == ButtonSensor.ButtonState.PRESSED ? ButtonSensor.ButtonState.RELEASED : ButtonSensor.ButtonState.PRESSED);
        reportMeasurement(measurement);

        // send current state
        measurement.setState(buttonSensor.getButtonState());
        reportMeasurement(measurement);
    }

}
