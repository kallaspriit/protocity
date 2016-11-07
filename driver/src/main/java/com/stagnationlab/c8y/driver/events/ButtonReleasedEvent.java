package com.stagnationlab.c8y.driver.events;

import com.cumulocity.rest.representation.event.EventRepresentation;

import java.util.Date;

public class ButtonReleasedEvent extends EventRepresentation {

    public ButtonReleasedEvent() {
        setType("ButtonReleasedEvent");
        setText("Button released");
        setTime(new Date());
    }
}
