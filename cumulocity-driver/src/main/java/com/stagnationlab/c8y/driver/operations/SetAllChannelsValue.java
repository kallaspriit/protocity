package com.stagnationlab.c8y.driver.operations;

import java.io.Serializable;

@SuppressWarnings("unused")
public class SetAllChannelsValue implements Serializable {

	private float value;

	public float getValue() {
		return value;
	}

	public void setValue(float value) {
		this.value = value;
	}

}
