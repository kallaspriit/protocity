package com.stagnationlab.c8y.driver.devices.simulated;

import com.stagnationlab.c8y.driver.devices.AbstractRelayActuator;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SimulatedRelayActuator extends AbstractRelayActuator {

    private static final Logger log = LoggerFactory.getLogger(SimulatedRelayActuator.class);

    public SimulatedRelayActuator(String id) {
        super(id);
    }

    @Override
    protected void applyRelayState(boolean isRelayClosed) {
        log.info("{} simulated relay", isRelayClosed ? "closing" : "opening");
    }
}
