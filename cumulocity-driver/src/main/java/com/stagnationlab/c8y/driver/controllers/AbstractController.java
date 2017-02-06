package com.stagnationlab.c8y.driver.controllers;

import java.util.Map;

import com.stagnationlab.c8y.driver.devices.AbstractDevice;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.etherio.Commander;

public abstract class AbstractController extends AbstractDevice implements EventBroker.EventBrokerListener {

	private final Map<String, Commander> commanders;
	final Config config;
	final EventBroker eventBroker;

	AbstractController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
		super(id);

		this.commanders = commanders;
		this.config = config;
		this.eventBroker = eventBroker;
	}

	public void handleEvent(String name, Object info) {}

	protected Commander getCommanderByName(String name) {
		if (!commanders.containsKey(name)) {
			return null;
		}

		return commanders.get(name);
	}


}
