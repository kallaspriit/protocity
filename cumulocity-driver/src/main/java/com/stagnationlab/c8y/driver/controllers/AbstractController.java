package com.stagnationlab.c8y.driver.controllers;

import java.util.List;
import java.util.Map;
import java.util.Properties;

import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.etherio.Commander;

import c8y.lx.driver.Driver;

public abstract class AbstractController {

	protected final Map<String, Commander> commanders;
	protected final Config config;

	public AbstractController(Map<String, Commander> commanders, Config config) {
		this.commanders = commanders;
		this.config = config;
	}

	public abstract List<Driver> setup();


}
