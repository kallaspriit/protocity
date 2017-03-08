package com.stagnationlab.c8y.driver.fragments.actuators;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

@SuppressWarnings({ "unused", "WeakerAccess" })
public class MultiDacActuator implements Serializable {

	private boolean isRunning;
	private int channelCount;
	private Map<Integer, Float> values;

	public MultiDacActuator() {
		reset();
	}

	public void reset() {
		isRunning = false;
		channelCount = 0;
		values = new HashMap<>();
	}

	public boolean getIsRunning() {
		return isRunning;
	}

	public void setIsRunning(boolean isRunning) {
		this.isRunning = isRunning;
	}

	public int getChannelCount() {
		return channelCount;
	}

	public synchronized void setChannelCount(int channelCount) {
		this.channelCount = channelCount;
		this.values = new HashMap<>();

		for (int i = 0; i < channelCount; i++) {
			updateChannelValue(i, 0);
		}
	}


	public synchronized Map<Integer, Float> getValues() {
		return values;
	}

	public synchronized void setValues(Map<Integer, Float> values) {
		this.values = values;
	}

	public synchronized void updateChannelValue(int channel, float value) {
		values.put(channel, value);
	}

}
