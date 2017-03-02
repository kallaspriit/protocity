package com.stagnationlab.c8y.driver.operations;

import java.util.HashMap;
import java.util.Map;

@SuppressWarnings("unused")
public class SetChannelValues {

	private Map<String, Number> values = new HashMap<>();

	public Map<String, Number> getValues() {
		return values;
	}

	public void setValues(Map<String, Number> values) {
		this.values = values;
	}

}
