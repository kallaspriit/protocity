package com.stagnationlab.c8y.driver.services;

import lombok.extern.slf4j.Slf4j;

@Slf4j
public class SimulatedVariance {

	private final float maxVelocity;
	private final float maxChangePercentage;
	private final float minVariance;
	private final float maxVariance;
	private float variance = 0.0f;
	private float currentVelocity = 0.0f;

	public SimulatedVariance(float maxVelocity, float maxChangePercentage, float minVariance, float maxVariance) {
		this.maxVelocity = maxVelocity;
		this.maxChangePercentage = maxChangePercentage;
		this.minVariance = minVariance;
		this.maxVariance = maxVariance;
	}

	public float getUpdatedVariance() {
		float maxChangeValue = maxVelocity * (maxChangePercentage / 100.0f);
		float deltaVelocity = Util.getRandomFloatInRange(-maxChangeValue, maxChangeValue);

		currentVelocity = Math.min(Math.max(currentVelocity + deltaVelocity, -maxVelocity), maxVelocity);
		variance = Math.min(Math.max(variance + currentVelocity, minVariance), maxVariance);

		boolean isVariancePeaked = variance == minVariance || variance == maxVariance;

		// reset velocity if variance peaks
		if (isVariancePeaked) {
			currentVelocity = 0;
		}

		log.trace("updated variance: {} (max change value: {}, delta velocity: {}, current velocity: {}, is peaked: {})", variance, maxChangeValue, deltaVelocity, currentVelocity, isVariancePeaked ? "yes" : "no");

		return variance;
	}

}
