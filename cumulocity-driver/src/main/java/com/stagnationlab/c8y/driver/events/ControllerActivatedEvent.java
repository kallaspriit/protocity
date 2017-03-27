package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class ControllerActivatedEvent extends EventRepresentation {

	public ControllerActivatedEvent() {
		setType("ControllerActivatedEvent");
		setText("Controller was activated");
		setTime(new Date());
	}
}
