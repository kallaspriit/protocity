package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class TagExitEvent  extends EventRepresentation {

	public TagExitEvent() {
		setType("TagExitEvent");
		setText("Tag exited");
		setTime(new Date());
	}
}
