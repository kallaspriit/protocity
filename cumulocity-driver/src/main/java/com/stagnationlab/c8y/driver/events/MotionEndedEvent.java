package com.stagnationlab.c8y.driver.events;

import com.cumulocity.rest.representation.event.EventRepresentation;

import java.util.Date;

public class MotionEndedEvent extends EventRepresentation {

    public MotionEndedEvent() {
        setType("MotionEndedEvent");
        setText("Motion ended");
        setTime(new Date());
    }

}
