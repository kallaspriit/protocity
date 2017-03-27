package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class ControllerDeactivatedEvent extends EventRepresentation {

	public ControllerDeactivatedEvent() {
		setType("ControllerDeactivatedEvent");
		setText("Controller was deactivated");
		setTime(new Date());
	}
}