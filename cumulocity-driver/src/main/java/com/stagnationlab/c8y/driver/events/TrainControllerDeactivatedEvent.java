package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class TrainControllerDeactivatedEvent extends EventRepresentation {

	public TrainControllerDeactivatedEvent() {
		setType("TrainControllerDeactivatedEvent");
		setText("Train controller was deactivated");
		setTime(new Date());
	}
}