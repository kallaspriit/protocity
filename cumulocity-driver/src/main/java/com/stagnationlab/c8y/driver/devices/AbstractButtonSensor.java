package com.stagnationlab.c8y.driver.devices;

import com.stagnationlab.c8y.driver.events.ButtonPressedEvent;
import com.stagnationlab.c8y.driver.events.ButtonReleasedEvent;
import com.stagnationlab.c8y.driver.fragments.sensors.ButtonSensor;
import com.stagnationlab.c8y.driver.measurements.ButtonStateMeasurement;

public abstract class AbstractButtonSensor extends AbstractDevice {

    private final ButtonSensor buttonSensor = new ButtonSensor();

    protected AbstractButtonSensor(String id) {
        super(id);
    }

    @Override
    protected String getType() {
        return buttonSensor.getClass().getSimpleName();
    }

    @Override
    protected Object getSensorFragment() {
        return buttonSensor;
    }

    protected void setIsButtonPressed(boolean isButtonPressed) {
        buttonSensor.setButtonState(isButtonPressed ? ButtonSensor.ButtonState.PRESSED : ButtonSensor.ButtonState.RELEASED);

        reportEvent(isButtonPressed ? new ButtonPressedEvent() : new ButtonReleasedEvent());
        updateState(buttonSensor);
        sendMeasurement();
    }

    private void sendMeasurement() {
        ButtonStateMeasurement measurement = new ButtonStateMeasurement();

        // send inverse measurement first to get a square graph
        measurement.setState(buttonSensor.getButtonState() == ButtonSensor.ButtonState.PRESSED ? ButtonSensor.ButtonState.RELEASED : ButtonSensor.ButtonState.PRESSED);
        reportMeasurement(measurement);

        // send current state
        measurement.setState(buttonSensor.getButtonState());
        reportMeasurement(measurement);
    }

}
