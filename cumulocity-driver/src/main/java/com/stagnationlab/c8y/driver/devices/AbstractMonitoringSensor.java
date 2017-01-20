package com.stagnationlab.c8y.driver.devices;


import com.stagnationlab.c8y.driver.fragments.MonitoringSensor;

public abstract class AbstractMonitoringSensor extends AbstractDevice {

    private final MonitoringSensor monitoringSensor = new MonitoringSensor();

	protected AbstractMonitoringSensor(String id) {
		super(id);
	}

	@Override
	protected String getType() {
		return monitoringSensor.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return monitoringSensor;
	}
}
