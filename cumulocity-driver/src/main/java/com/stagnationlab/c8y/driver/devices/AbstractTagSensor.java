package com.stagnationlab.c8y.driver.devices;

import java.util.ArrayList;
import java.util.List;

import com.stagnationlab.c8y.driver.events.TagEnterEvent;
import com.stagnationlab.c8y.driver.events.TagExitEvent;
import com.stagnationlab.c8y.driver.fragments.sensors.TagSensor;

public class AbstractTagSensor extends AbstractDevice {

	public interface TagEventListener {
		void onTagEnter(String tagName);
		void onTagExit();
	}

	protected final TagSensor state = new TagSensor();
	private final List<TagEventListener> tagEventListeners = new ArrayList<>();

	protected AbstractTagSensor(String id) {
		super(id);
	}

	@Override
	protected String getType() {
		return state.getClass().getSimpleName();
	}

	public void addTagEventListener(TagEventListener tagEventListener) {
		tagEventListeners.add(tagEventListener);
	}

	protected void emitTagEnter(String tagName) {
		for (TagEventListener tagEventListener : tagEventListeners) {
			tagEventListener.onTagEnter(tagName);
		}

		state.setTagName(tagName);
		state.setTagActive(true);

		reportEvent(new TagEnterEvent());
		updateState(state);
	}

	protected void emitTagExit() {
		for (TagEventListener tagEventListener : tagEventListeners) {
			tagEventListener.onTagExit();
		}

		state.setTagActive(false);

		reportEvent(new TagExitEvent());
		updateState(state);
	}

}
