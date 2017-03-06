package com.stagnationlab.c8y.driver.fragments.controllers;

@SuppressWarnings("unused")
public class Weather {

	private boolean isConnected;
	private float temperature;
	private float lightLevel;
	private float humidity;
	private float pressure;
	private float soundLevel;

	public void reset() {
		isConnected = false;
		temperature = 0.0f;
		lightLevel = 0.0f;
		humidity = 0.0f;
		pressure = 0.0f;
		soundLevel = 0.0f;
	}

	public boolean getIsConnected() {
		return isConnected;
	}

	public void setIsConnected(boolean isConnected) {
		this.isConnected = isConnected;
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
