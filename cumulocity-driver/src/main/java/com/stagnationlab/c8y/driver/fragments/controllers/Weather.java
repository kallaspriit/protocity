package com.stagnationlab.c8y.driver.fragments.controllers;

@SuppressWarnings("unused")
public class Weather {

	private boolean isRunning;
	private float temperature;
	private float lightLevel;
	private float humidity;
	private float pressure;
	private float soundLevel;

	public void reset() {
		isRunning = false;
		temperature = 0.0f;
		lightLevel = 0.0f;
		humidity = 0.0f;
		pressure = 0.0f;
		soundLevel = 0.0f;
	}

	public boolean getIsRunning() {
		return isRunning;
	}

	public void setIsRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}

	public float getTemperature() {
		return temperature;
	}

	public void setTemperature(float temperature) {
		this.temperature = temperature;
	}

	public float getLightLevel() {
		return lightLevel;
	}

	public void setLightLevel(float lightLevel) {
		this.lightLevel = lightLevel;
	}

	public float getHumidity() {
		return humidity;
	}

	public void setHumidity(float humidity) {
		this.humidity = humidity;
	}

	public float getPressure() {
		return pressure;
	}

	public void setPressure(float pressure) {
		this.pressure = pressure;
	}

	public float getSoundLevel() {
		return soundLevel;
	}

	public void setSoundLevel(float soundLevel) {
		this.soundLevel = soundLevel;
	}
}
