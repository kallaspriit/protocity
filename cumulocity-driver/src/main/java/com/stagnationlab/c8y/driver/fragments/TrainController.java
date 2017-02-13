package com.stagnationlab.c8y.driver.fragments;

@SuppressWarnings("unused")
public class TrainController {

	private float batteryVoltage = 0.0f;
	private int chargePercentage = 0;
	private boolean isObstacleDetected = false;
	private float obstacleDistance = 0.0f;
	private float realSpeed = 0.0f;
	private float targetSpeed = 0.0f;

	public float getBatteryVoltage() {
		return batteryVoltage;
	}

	public void setBatteryVoltage(float batteryVoltage) {
		this.batteryVoltage = batteryVoltage;
	}

	public int getChargePercentage() {
		return chargePercentage;
	}

	public void setChargePercentage(int chargePercentage) {
		this.chargePercentage = chargePercentage;
	}

	public boolean getIsObstacleDetected() {
		return isObstacleDetected;
	}

	public void setIsObstacleDetected(boolean obstacleDetected) {
		isObstacleDetected = obstacleDetected;
	}

	public float getObstacleDistance() {
		return obstacleDistance;
	}

	public void setObstacleDistance(float obstacleDistance) {
		this.obstacleDistance = obstacleDistance;
	}

	public float getRealSpeed() {
		return realSpeed;
	}

	public void setRealSpeed(float realSpeed) {
		this.realSpeed = realSpeed;
	}

	public float getTargetSpeed() {
		return targetSpeed;
	}

	public void setTargetSpeed(float targetSpeed) {
		this.targetSpeed = targetSpeed;
	}
}
