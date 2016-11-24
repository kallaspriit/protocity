package com.stagnationlab.c8y.driver.platforms.etherio;

import com.stagnationlab.c8y.driver.devices.AbstractLightSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class EtherioLightSensor extends AbstractLightSensor {

    private final Commander commander;
    private final int portNumber;
    private PortController portController;

    public EtherioLightSensor(String id, Commander commander, int portNumber) {
        super(id);

        this.commander = commander;
        this.portNumber = portNumber;
    }

    @Override
    public void initialize() throws Exception {
        super.initialize();

        portController = new PortController(portNumber, commander);
        portController.setPortMode(PortController.PortMode.ANALOG_IN);
    }

    @Override
    public void start() {
        super.start();

        portController.listenAnalogValueChange(0.01f, 5000, new PortController.PortEventListener() {

            @Override
            public void onPortAnalogValueChange(int id, float value) {
                reportIlluminance(value);
            }
        });
    }

}