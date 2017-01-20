package com.stagnationlab.c8y.driver.fragments;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

import org.svenson.JSONProperty;

@SuppressWarnings({ "unused", "WeakerAccess" })
public class MultiDacActuator implements Serializable {

	private static final long serialVersionUID = 2702169573343647512L;
	private int channelCount = 0;
	private Map<Integer, Float> values = new HashMap<>();

	public MultiDacActuator() {

	}

	public MultiDacActuator(int channelCount) {
		setChannelCount(channelCount);
	}

	public int getChannelCount() {
		return channelCount;
	}

	public void setChannelCount(int channelCount) {
		this.channelCount = channelCount;
		this.values = new HashMap<>();

		for (int i = 0; i < channelCount; i++) {
			updateChannelValue(i, 0);
		}
	}


	public Map<Integer, Float> getValues() {
		return values;
	}

	public void setValues(Map<Integer, Float> values) {
		this.values = values;
	}

	/*
	public float getChannelValue(int channel) {
		return values.get(channel);
	}
	*/

	public void updateChannelValue(int channel, float value) {
		values.put(channel, value);
	}

	public int hashCode() {
		return values.hashCode();
	}

	public boolean equals(Object other) {
		if (this == other) {
			return true;
		} else if (!(other instanceof MultiDacActuator)) {
			return false;
		} else {
			MultiDacActuator that = (MultiDacActuator)other;

			Map<Integer, Float> myValues = getValues();
			Map<Integer, Float> otherValues = that.getValues();

			if (otherValues.size() != myValues.size()) {
				return false;
			}

			for (int i = 0; i < myValues.size(); i++) {
				if (!Objects.equals(myValues.get(i), otherValues.get(i))) {
					return false;
				}
			}

			return true;
		}
	}

}
