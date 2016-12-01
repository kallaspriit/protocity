package com.stagnationlab.c8y.driver.devices;

import com.stagnationlab.c8y.driver.events.MotionDetectedEvent;
import com.stagnationlab.c8y.driver.events.MotionEndedEvent;
import com.stagnationlab.c8y.driver.fragments.MotionSensor;
import com.stagnationlab.c8y.driver.measurements.MotionStateMeasurement;

public abstract class AbstractMotionSensor extends AbstractDevice {

	private final MotionSensor motionSensor = new MotionSensor();

    protected AbstractMotionSensor(String id) {
	    super(id);
    }

	@Override
	protected String getType() {
		return motionSensor.getClass().getSimpleName();
	}

	protected void setIsMotionDetected(boolean isMotionDetected) {
    	motionSensor.setMotionState(isMotionDetected ? MotionSensor.MotionState.MOTION_DETECTED : MotionSensor.MotionState.MOTION_NOT_DETECTED);

        reportEvent(isMotionDetected ? new MotionDetectedEvent() : new MotionEndedEvent());
        updateState(motionSensor);
        sendMeasurement();
    }

	private void sendMeasurement() {
		MotionStateMeasurement measurement = new MotionStateMeasurement();

		// send inverse measurement first to get a square graph
		measurement.setState(motionSensor.getMotionState() == MotionSensor.MotionState.MOTION_DETECTED ? MotionSensor.MotionState.MOTION_NOT_DETECTED : MotionSensor.MotionState.MOTION_DETECTED);
		reportMeasurement(measurement);

		// send current state
		measurement.setState(motionSensor.getMotionState());
		reportMeasurement(measurement);
	}
}
