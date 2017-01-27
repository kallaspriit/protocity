package com.stagnationlab.c8y.driver.operations;

import java.math.BigDecimal;
import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@SuppressWarnings("unused")
public class SetChannelValues {

	private static final Logger log = LoggerFactory.getLogger(SetChannelValues.class);

	private Map<String, Number> values = new HashMap<>();

	public Map<String, Number> getValues() {
		return values;
	}

	public void setValues(Map<String, Number> values) {
		this.values = values;
	}

}
