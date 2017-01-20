package com.stagnationlab.c8y.driver.events;

import java.util.Date;

import com.cumulocity.rest.representation.event.EventRepresentation;

public class TagEnterEvent extends EventRepresentation {

	public TagEnterEvent() {
		setType("TagEnterEvent");
		setText("Tag entered");
		setTime(new Date());
	}
}
