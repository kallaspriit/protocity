package com.stagnationlab.c8y.driver.actuators;

import c8y.Hardware;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class SimulatedRelayActuator extends AbstractRelayActuator {

    private static final Logger log = LoggerFactory.getLogger(SimulatedRelayActuator.class);

    public SimulatedRelayActuator(String id) {
        super(id);
    }

    @Override
    Hardware getHardware() {
        return new Hardware(
                "Simulated Relay Actuator",
                "356734556743235",
                "1.0.0"
        );
    }

    @Override
    void applyRelayState(boolean isRelayOn) {
        log.info("turning simulated relay " + (isRelayOn ? "on" : "off"));
    }
}
