package com.stagnationlab.c8y.driver.controllers;

import java.util.Map;

import com.stagnationlab.c8y.driver.devices.AbstractDevice;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.etherio.Commander;

public abstract class AbstractController extends AbstractDevice {

	protected final Map<String, Commander> commanders;
	protected final Config config;

	public AbstractController(String id, Map<String, Commander> commanders, Config config) {
		super(id);

		this.commanders = commanders;
		this.config = config;
	}


}
