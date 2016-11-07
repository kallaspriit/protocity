package com.stagnationlab.c8y.driver.events;

import com.cumulocity.rest.representation.event.EventRepresentation;

import java.util.Date;

public class ButtonPressedEvent extends EventRepresentation {

    public ButtonPressedEvent() {
        setType("ButtonPressedEvent");
        setText("Button pressed");
        setTime(new Date());
    }
}
