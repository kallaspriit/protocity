package com.stagnationlab.c8y.driver.devices.simulated;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.devices.AbstractRelayActuator;

@SuppressWarnings("unused")
@Slf4j
public class SimulatedRelayActuator extends AbstractRelayActuator {

    public SimulatedRelayActuator(String id) {
        super(id);
    }

    @Override
    protected void applyRelayState(boolean isRelayClosed) {
        log.info("{} simulated relay", isRelayClosed ? "closing" : "opening");
    }
}
