package com.stagnationlab.c8y.driver.devices.etherio;

import com.stagnationlab.c8y.driver.devices.AbstractRelayActuator;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class EtherioRelayActuator extends AbstractRelayActuator {

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
        super.initialize();

        portController = new PortController(portNumber, commander);
        portController.setPortMode(PortController.PortMode.DIGITAL_OUT);
    }

    @Override
    protected void applyRelayState(boolean isRelayClosed) {
        PortController.DigitalValue digitalValue = isRelayClosed ? PortController.DigitalValue.HIGH : PortController.DigitalValue.LOW;

        portController.setDigitalValue(digitalValue);
    }
}