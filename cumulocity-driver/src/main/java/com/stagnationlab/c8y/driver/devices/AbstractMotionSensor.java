package com.stagnationlab.c8y.driver.devices;

import java.util.ArrayList;
import java.util.List;

import com.stagnationlab.c8y.driver.events.MotionDetectedEvent;
import com.stagnationlab.c8y.driver.events.MotionEndedEvent;
import com.stagnationlab.c8y.driver.fragments.sensors.MotionSensor;

public abstract class AbstractMotionSensor extends AbstractDevice {

	public interface Listener {
		void onMotionChange(boolean isMotionDetected);
	}

	protected final MotionSensor state = new MotionSensor();
	private final List<Listener> listeners = new ArrayList<>();

    protected AbstractMotionSensor(String id) {
	    super(id);
    }

	@Override
	protected String getType() {
		return state.getClass().getSimpleName();
	}

	public void addListener(Listener listener) {
		listeners.add(listener);
	}

	protected void setIsMotionDetected(boolean isMotionDetected) {
		for (Listener listener : listeners) {
			listener.onMotionChange(isMotionDetected);
		}

		reportEvent(isMotionDetected ? new MotionDetectedEvent() : new MotionEndedEvent());

		state.setMotionState(isMotionDetected ? MotionSensor.MotionState.MOTION_DETECTED : MotionSensor.MotionState.MOTION_NOT_DETECTED);
		updateState(state);
        //sendMeasurement();
    }

    /*
	private void sendMeasurement() {
		MotionStateMeasurement measurement = new MotionStateMeasurement();

		// send inverse measurement first to get a square graph
		measurement.setState(state.getMotionState() == MotionSensor.MotionState.MOTION_DETECTED ? MotionSensor.MotionState.MOTION_NOT_DETECTED : MotionSensor.MotionState.MOTION_DETECTED);
		reportMeasurement(measurement);

		// send current state
		measurement.setState(state.getMotionState());
		reportMeasurement(measurement);
	}
	*/
}
