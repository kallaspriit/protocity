package com.stagnationlab.c8y.driver.fragments;

import java.util.ArrayList;
import java.util.List;

@SuppressWarnings("unused")
public class Gateway {

	private String version;
	private boolean isRunning;
	private List<Controller> controllers;

	public Gateway() {
		reset();
	}

	public void reset() {
		version = "";
		isRunning = false;
		controllers = new ArrayList<>();
	}

	public String getVersion() {
		return version;
	}

	public void setVersion(String version) {
		this.version = version;
	}

	public boolean getIsRunning() {
		return isRunning;
	}

	public void setIsRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}

	public synchronized List<Controller> getControllers() {
		return controllers;
	}

	public synchronized void setControllers(List<Controller> controllers) {
		this.controllers = controllers;
	}

	public synchronized void addController(Controller controller) {
		controllers.add(controller);
	}

	public synchronized Controller controllerByName(String name) {
		return controllers.stream().filter(controller -> controller.getName().equals(name)).findFirst().orElse(null);
	}
}
