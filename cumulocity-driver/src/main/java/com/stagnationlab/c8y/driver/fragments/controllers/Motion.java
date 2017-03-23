package com.stagnationlab.c8y.driver.fragments.controllers;

public class Motion {

	private boolean isRunning;

	public Motion() {
		reset();
	}

	public void reset() {
		isRunning = false;
	}

	public boolean getIsRunning() {
		return isRunning;
	}

	public void setIsRunning(boolean running) {
		isRunning = running;
	}

}
