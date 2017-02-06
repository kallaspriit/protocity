package com.stagnationlab.c8y.driver.services;

import java.util.ArrayList;
import java.util.List;

public class EventBroker {

	public interface EventBrokerListener {
		void handleEvent(String name, Object info);
	}

	private List<EventBrokerListener> listeners = new ArrayList<>();

	public void addListener(EventBrokerListener listener) {
		listeners.add(listener);
	}

	public void emitEvent(String name, Object info) {
		listeners.forEach(listener -> listener.handleEvent(name, info));
	}

	public void emitEvent(String name) {
		emitEvent(name, null);
	}

}
