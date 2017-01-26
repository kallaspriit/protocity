package com.stagnationlab.c8y.driver.services;

import java.util.Arrays;
import java.util.List;
import java.util.Properties;
import java.util.stream.Collectors;

public class Config extends Properties {

	public String getString(String name, String defaultValue) {
		Object value = get(name);

		if (value == null) {
			return defaultValue;
		}

		return value.toString();
	}

	public String getString(String name) {
		Object value = get(name);

		if (value == null) {
			throw new IllegalArgumentException("configuration parameter called '" + name + "' does not exist");
		}

		return value.toString();
	}

	public int getInt(String name, int defaultValue) {
		Object value = get(name);

		if (value == null) {
			return defaultValue;
		}

		return Integer.valueOf(value.toString());
	}

	public int getInt(String name) {
		Object value = get(name);

		if (value == null) {
			throw new IllegalArgumentException("configuration parameter called '" + name + "' does not exist");
		}

		return Integer.valueOf(value.toString());
	}

	public List<String> getStringArray(String name) {
		String values = getString(name);

		return Arrays.stream(values.split(",")).map(value -> value.trim()).collect(Collectors.toList());
	}
}
