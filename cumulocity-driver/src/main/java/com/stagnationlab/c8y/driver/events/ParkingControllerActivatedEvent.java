package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class ParkingControllerActivatedEvent extends EventRepresentation {

	public ParkingControllerActivatedEvent() {
		setType("ParkingControllerActivatedEvent");
		setText("Parking controller was activated");
		setTime(new Date());
	}
}