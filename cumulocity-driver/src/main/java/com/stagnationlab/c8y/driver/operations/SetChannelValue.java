package com.stagnationlab.c8y.driver.operations;

import java.io.Serializable;

@SuppressWarnings("unused")
public class SetChannelValue implements Serializable {

	private int channel;
	private float value;

	public int getChannel() {
		return channel;
	}

	public void setChannel(int channel) {
		this.channel = channel;
	}

	public float getValue() {
		return value;
	}

	public void setValue(float value) {
		this.value = value;
	}
}
