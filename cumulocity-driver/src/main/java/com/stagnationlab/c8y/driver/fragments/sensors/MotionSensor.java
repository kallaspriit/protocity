package com.stagnationlab.c8y.driver.fragments.sensors;

public class MotionSensor {

	public enum MotionState {
		MOTION_NOT_DETECTED,
		MOTION_DETECTED
	}

	private MotionSensor.MotionState motionState;

	public MotionSensor.MotionState getMotionState() {
		return motionState;
	}

	public void setMotionState(MotionSensor.MotionState motionState) {
		this.motionState = motionState;
	}

	public int hashCode() {
		return motionState != null ? motionState.hashCode() : 0;
	}

	public boolean equals(Object other) {
		if(this == other) {
			return true;
		} else if(!(other instanceof MotionSensor)) {
			return false;
		} else {
			MotionSensor motionSensor = (MotionSensor)other;

			return motionState == motionSensor.motionState;
		}
	}
}
