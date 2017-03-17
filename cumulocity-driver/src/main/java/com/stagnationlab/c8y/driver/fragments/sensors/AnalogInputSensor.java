package com.stagnationlab.c8y.driver.fragments.sensors;

public class AnalogInputSensor {

	private boolean isRunning = false;
	private float value = 0;

	public boolean getIsRunning() {
		return isRunning;
	}

	public void setIsRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}

	public void setValue(float value) {
		this.value = value;
	}

	public float getValue() {
		return value;
	}

}
