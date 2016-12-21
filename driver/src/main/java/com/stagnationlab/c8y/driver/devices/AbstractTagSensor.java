package com.stagnationlab.c8y.driver.devices;

import com.stagnationlab.c8y.driver.events.TagEnterEvent;
import com.stagnationlab.c8y.driver.events.TagExitEvent;
import com.stagnationlab.c8y.driver.fragments.TagSensor;

public class AbstractTagSensor extends AbstractDevice {

	private final TagSensor tagSensor = new TagSensor();

	protected AbstractTagSensor(String id) {
		super(id);
	}

	@Override
	protected String getType() {
		return tagSensor.getClass().getSimpleName();
	}

	protected void emitTagEnter(String tagName) {
		tagSensor.setTagName(tagName);
		tagSensor.setTagActive(true);

		reportEvent(new TagEnterEvent());
		updateState(tagSensor);
	}

	protected void emitTagExit() {
		tagSensor.setTagActive(false);

		reportEvent(new TagExitEvent());
		updateState(tagSensor);
	}

}
