package com.stagnationlab.c8y.driver.fragments;

@SuppressWarnings("unused")
public class LightingController {

	private float detectedLightLevel = 0.0f;
	private float outputLightLevel = 0.0f;

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
