package com.stagnationlab.c8y.driver.devices;

import java.util.ArrayList;
import java.util.List;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.fragments.sensors.AnalogInputSensor;
import com.stagnationlab.c8y.driver.measurements.AnalogMeasurement;

@Slf4j
public class AbstractAnalogInputSensor extends AbstractDevice {

	public interface Listener {
		void onValueChange(float value);
	}

	protected final AnalogInputSensor state = new AnalogInputSensor();
	protected final String unit;
	private final List<Listener> listeners = new ArrayList<>();

	protected AbstractAnalogInputSensor(String id, String unit) {
		super(id);

		this.unit = unit;
	}

	@Override
	protected String getType() {
		return state.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return state;
	}

	public AnalogInputSensor getState() {
		return state;
	}

	public void addListener(Listener listener) {
		listeners.add(listener);
	}

	protected void handleValueChange(float value) {
		log.trace("analog input '{}' value changed to {}", id, value);

		for (Listener listener : listeners) {
			listener.onValueChange(value);
		}

		state.setValue(value);
		updateState(state);

		sendMeasurement();
	}

	private void sendMeasurement() {
		AnalogMeasurement measurement = new AnalogMeasurement(state.getValue(), unit);

		reportMeasurement(measurement);
	}

}
