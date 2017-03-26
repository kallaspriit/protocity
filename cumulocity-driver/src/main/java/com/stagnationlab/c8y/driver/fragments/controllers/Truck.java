package com.stagnationlab.c8y.driver.fragments.controllers;

@SuppressWarnings("unused")
public class Truck {

	private float batteryVoltage;
	private int batteryChargePercentage;
	private float chargePower;
	private float gridPowerBalance;
	private boolean isRunning;
	private boolean isCharging;

	public Truck() {
		reset();
	}

	public void reset() {
		batteryVoltage = 0.0f;
		batteryChargePercentage = 0;
		chargePower = 0.0f;
		isRunning = false;
		isCharging = false;
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

	public float getChargePower() {
		return chargePower;
	}

	public void setChargePower(float chargePower) {
		this.chargePower = chargePower;
	}

	public float getGridPowerBalance() {
		return gridPowerBalance;
	}

	public void setGridPowerBalance(float gridPowerBalance) {
		this.gridPowerBalance = gridPowerBalance;
	}

	public boolean getIsRunning() {
		return isRunning;
	}

	public void setIsRunning(boolean running) {
		isRunning = running;
	}

	public boolean getIsCharging() {
		return isCharging;
	}

	public void setIsCharging(boolean charging) {
		isCharging = charging;
	}
}
