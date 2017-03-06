package com.stagnationlab.c8y.driver.fragments.controllers;

@SuppressWarnings("unused")
public class Lighting {

	private boolean isRunning;
	private float detectedLightLevel;
	private float outputLightLevel;

	public Lighting() {
		reset();
	}

	public void reset() {
		isRunning = false;
		detectedLightLevel = 0.0f;
		outputLightLevel = 0.0f;
	}

	public boolean getIsRunning() {
		return isRunning;
	}

	public void setIsRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}

	public float getDetectedLightLevel() {
		return detectedLightLevel;
	}

	public void setDetectedLightLevel(float detectedLightLevel) {
		this.detectedLightLevel = detectedLightLevel;
	}

	public float getOutputLightLevel() {
		return outputLightLevel;
	}

	public void setOutputLightLevel(float outputLightLevel) {
		this.outputLightLevel = outputLightLevel;
	}
}
