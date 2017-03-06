package com.stagnationlab.c8y.driver.fragments.controllers;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

@SuppressWarnings("unused")
public class Train {

	private float batteryVoltage;
	private int batteryChargePercentage;
	private boolean isRunning;
	private boolean isCharging;
	private boolean isObstacleDetected;
	private boolean isInStation;
	private float obstacleDistance;
	private float realSpeed;
	private float targetSpeed;
	private List<String> operations;
	private int currentOperationIndex;
	private String nextStationName;
	private String previousStationName;
	private long chargeStartTimestamp;
	// TODO keep track of number of tickets bought

	public Train() {
		reset();
	}

	public void reset() {
		batteryVoltage = 0.0f;
		batteryChargePercentage = 0;
		isRunning = false;
		isCharging = false;
		isObstacleDetected = false;
		isInStation = false;
		obstacleDistance = 0.0f;
		realSpeed = 0.0f;
		targetSpeed = 0.0f;
		operations = new ArrayList<>();
		currentOperationIndex = 0;
		nextStationName = "";
		previousStationName = "";
		chargeStartTimestamp = 0;
	}

	public float getBatteryVoltage() {
		return batteryVoltage;
	}

	public void setBatteryVoltage(float batteryVoltage) {
		this.batteryVoltage = batteryVoltage;
	}

	public int getBatteryChargePercentage() {
		return batteryChargePercentage;
	}

	public void setBatteryChargePercentage(int batteryChargePercentage) {
		this.batteryChargePercentage = batteryChargePercentage;
	}

	public boolean getIsRunning() {
		return isRunning;
	}

	public void setIsRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}

	public boolean getIsObstacleDetected() {
		return isObstacleDetected;
	}

	public void setIsObstacleDetected(boolean isObstacleDetected) {
		this.isObstacleDetected = isObstacleDetected;
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

	public synchronized List<String> getOperations() {
		return operations;
	}

	public synchronized void setOperations(List<String> operations) {
		this.operations = operations;
	}

	public int getCurrentOperationIndex() {
		return currentOperationIndex;
	}

	public void setCurrentOperationIndex(int currentOperationIndex) {
		this.currentOperationIndex = currentOperationIndex;
	}

	public boolean getIsInStation() {
		return isInStation;
	}

	public void setIsInStation(boolean isInStation) {
		this.isInStation = isInStation;
	}

	public boolean getIsCharging() {
		return isCharging;
	}

	public void setIsCharging(boolean isCharging) {
		if (isCharging && !this.isCharging) {
			chargeStartTimestamp = (new Date()).getTime();
		}

		this.isCharging = isCharging;
	}

	public String getNextStationName() {
		return nextStationName;
	}

	public void setNextStationName(String nextStationName) {
		this.nextStationName = nextStationName;
	}

	public String getPreviousStationName() {
		return previousStationName;
	}

	public void setPreviousStationName(String previousStationName) {
		this.previousStationName = previousStationName;
	}

	public long getChargeStartTimestamp() {
		return chargeStartTimestamp;
	}

	public void setChargeStartTimestamp(long chargeStartTimestamp) {
		this.chargeStartTimestamp = chargeStartTimestamp;
	}
}
