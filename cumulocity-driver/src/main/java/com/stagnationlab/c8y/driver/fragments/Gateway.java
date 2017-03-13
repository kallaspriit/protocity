package com.stagnationlab.c8y.driver.fragments;

@SuppressWarnings("unused")
public class Gateway {

	private String version;
	private boolean isRunning;

	public Gateway() {
		reset();
	}

	public void reset() {
		version = "";
		isRunning = false;
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
}
