package com.stagnationlab.c8y.driver.fragments;

public class ButtonSensor {

    public enum ButtonState {
        RELEASED,
        PRESSED
    }

    private ButtonState buttonState;

    public ButtonState getButtonState() {
        return buttonState;
    }

    public void setButtonState(ButtonState buttonState) {
        this.buttonState = buttonState;
    }

    public int hashCode() {
        return buttonState != null ? buttonState.hashCode() : 0;
    }

    public boolean equals(Object other) {
        if(this == other) {
            return true;
        } else if(!(other instanceof ButtonSensor)) {
            return false;
        } else {
            ButtonSensor buttonSensor = (ButtonSensor)other;

            return buttonState == buttonSensor.buttonState;
        }
    }

}
