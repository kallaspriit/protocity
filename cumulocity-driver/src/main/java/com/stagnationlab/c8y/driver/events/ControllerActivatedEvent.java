package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class ControllerActivatedEvent extends EventRepresentation {

	public ControllerActivatedEvent(String controllerName) {
		setType("ControllerActivatedEvent");
		setText(controllerName);
		setTime(new Date());
	}
}
