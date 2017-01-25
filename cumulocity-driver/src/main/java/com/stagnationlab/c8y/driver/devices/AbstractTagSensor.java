package com.stagnationlab.c8y.driver.devices;

import java.util.ArrayList;
import java.util.List;

import com.stagnationlab.c8y.driver.events.TagEnterEvent;
import com.stagnationlab.c8y.driver.events.TagExitEvent;
import com.stagnationlab.c8y.driver.fragments.TagSensor;

public class AbstractTagSensor extends AbstractDevice {

	public interface TagEventListener {
		void onTagEnter(String tagName);
		void onTagExit();
	}

	private final TagSensor tagSensor = new TagSensor();
	private final List<TagEventListener> tagEventListeners = new ArrayList<>();

	protected AbstractTagSensor(String id) {
		super(id);
	}

	@Override
	protected String getType() {
		return tagSensor.getClass().getSimpleName();
	}

	public void addTagEventListener(TagEventListener tagEventListener) {
		tagEventListeners.add(tagEventListener);
	}

	protected void emitTagEnter(String tagName) {
		for (TagEventListener tagEventListener : tagEventListeners) {
			tagEventListener.onTagEnter(tagName);
		}

		tagSensor.setTagName(tagName);
		tagSensor.setTagActive(true);

		reportEvent(new TagEnterEvent());
		updateState(tagSensor);
	}

	protected void emitTagExit() {
		for (TagEventListener tagEventListener : tagEventListeners) {
			tagEventListener.onTagExit();
		}

		tagSensor.setTagActive(false);

		reportEvent(new TagExitEvent());
		updateState(tagSensor);
	}

}
