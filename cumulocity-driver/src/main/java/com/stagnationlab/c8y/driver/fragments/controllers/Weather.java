package com.stagnationlab.c8y.driver.fragments.controllers;

@SuppressWarnings("unused")
public class Weather {

	private float temperature = 0.0f;
	private float lightLevel = 0.0f;
	private float humidity = 0.0f;
	private float pressure = 0.0f;
	private float soundLevel = 0.0f;

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
