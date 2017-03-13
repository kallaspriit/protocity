package com.stagnationlab.c8y.driver.devices;


import com.stagnationlab.c8y.driver.fragments.sensors.MonitoringSensor;

public abstract class AbstractMonitoringSensor extends AbstractDevice {

    protected final MonitoringSensor state;

	protected AbstractMonitoringSensor(String id, String name, String host, int port) {
		super(id);

		state = new MonitoringSensor(name, host, port);
	}

	@Override
	protected String getType() {
		return state.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return state;
	}
}
