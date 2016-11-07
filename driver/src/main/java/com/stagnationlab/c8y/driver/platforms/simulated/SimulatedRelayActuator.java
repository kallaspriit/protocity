package com.stagnationlab.c8y.driver.platforms.simulated;

import c8y.Hardware;
import com.stagnationlab.c8y.driver.devices.AbstractRelayActuator;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SimulatedRelayActuator extends AbstractRelayActuator {

    private static final Logger log = LoggerFactory.getLogger(SimulatedRelayActuator.class);

    public SimulatedRelayActuator(String id) {
        super(id);
    }

    @Override
    protected Hardware getHardware() {
        return new Hardware(
                "Simulated Relay Actuator",
                "356734556743235",
                "1.0.0"
        );
    }

    @Override
    protected void applyRelayState(boolean isRelayOn) {
        log.info("turning simulated relay " + (isRelayOn ? "on" : "off"));
    }
}
