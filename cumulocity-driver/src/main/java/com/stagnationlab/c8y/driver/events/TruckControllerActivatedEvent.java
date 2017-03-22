package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class TruckControllerActivatedEvent extends EventRepresentation {

	public TruckControllerActivatedEvent() {
		setType("TruckControllerActivatedEvent");
		setText("Truck controller was activated");
		setTime(new Date());
	}
}