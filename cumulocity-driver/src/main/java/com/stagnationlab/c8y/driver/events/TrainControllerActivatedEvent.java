package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class TrainControllerActivatedEvent extends EventRepresentation {

	public TrainControllerActivatedEvent() {
		setType("TrainControllerActivatedEvent");
		setText("Train controller was activated");
		setTime(new Date());
	}
}