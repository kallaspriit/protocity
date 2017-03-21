package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class LightingControllerActivatedEvent extends EventRepresentation {

	public LightingControllerActivatedEvent() {
		setType("LightingControllerActivatedEvent");
		setText("Weather controller was activated");
		setTime(new Date());
	}
}
