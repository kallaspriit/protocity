package com.stagnationlab.c8y.driver.platforms.etherio;

import c8y.Hardware;
import com.stagnationlab.c8y.driver.devices.AbstractRelayActuator;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class EtherioRelayActuator extends AbstractRelayActuator {

    private static final Logger log = LoggerFactory.getLogger(EtherioRelayActuator.class);

    private final Commander commander;
    private final int portNumber;
    private PortController portController;

    public EtherioRelayActuator(String id, Commander commander, int portNumber) {
        super(id);

        this.commander = commander;
        this.portNumber = portNumber;
    }

    @Override
    public void initialize() throws Exception {
        log.info("initializing");

        portController = new PortController(portNumber, commander);
        portController.setPortMode(PortController.PortMode.DIGITAL_OUT);
    }

    @Override
    protected Hardware getHardware() {
        return new Hardware(
                "EtherIO Relay Actuator",
                "882347545672456",
                "1.0.0"
        );
    }

    @Override
    protected void applyRelayState(boolean isRelayClosed) {
        log.info("turning relay " + (isRelayClosed ? "on" : "off") + " on port " + portNumber);

        portController.setDigitalValue(isRelayClosed ? PortController.DigitalValue.HIGH : PortController.DigitalValue.LOW);
    }
}