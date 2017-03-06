package com.stagnationlab.c8y.driver.fragments.controllers;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

@SuppressWarnings("unused")
public class Train {

	private float batteryVoltage = 0.0f;
	private int batteryChargePercentage = 0;
	private boolean isConnected = false;
	private boolean isCharging = false;
	private boolean isObstacleDetected = false;
	private boolean isInStation = false;
	private float obstacleDistance = 0.0f;
	private float realSpeed = 0.0f;
	private float targetSpeed = 0.0f;
	private List<String> operations = new ArrayList<>();
	private int currentOperationIndex = 0;
	private String nextStationName = "";
	private String previousStationName = "";
	private long chargeStartTimestamp = 0;
	// TODO keep track of number of tickets bought

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

	public boolean getIsConnected() {
		return isConnected;
	}

	public void setIsConnected(boolean isConnected) {
		this.isConnected = isConnected;
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

	public List<String> getOperations() {
		return operations;
	}

	public void setOperations(List<String> operations) {
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
