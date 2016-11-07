package com.stagnationlab.c8y.driver.events;

import com.cumulocity.rest.representation.event.EventRepresentation;

import java.util.Date;

public class MotionDetectedEvent extends EventRepresentation {

    public MotionDetectedEvent() {
        setType("MotionDetectedEvent");
        setText("Motion detected");
        setTime(new Date());
    }
}
