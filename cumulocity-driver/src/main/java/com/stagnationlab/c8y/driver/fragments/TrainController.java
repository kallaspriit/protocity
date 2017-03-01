package com.stagnationlab.c8y.driver.fragments;

import java.util.ArrayList;
import java.util.List;

@SuppressWarnings("unused")
public class TrainController {

	private float batteryVoltage = 0.0f;
	private int chargePercentage = 0;
	private boolean isObstacleDetected = false;
	private float obstacleDistance = 0.0f;
	private float realSpeed = 0.0f;
	private float targetSpeed = 0.0f;
	private List<String> operations = new ArrayList<>();
	private int currentOperationIndex = 0;
	private boolean isInStation = false;
	private boolean isCharging = false;
	private String nextStationName = "";
	private String previousStationName = "";

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

	public void setIsCharging(boolean charging) {
		isCharging = charging;
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
}
