package com.stagnationlab.c8y.driver.platforms.etherio;

import java.util.List;

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
    }

    @Override
    public void start() {
        super.start();

        portController.sendPortCommand("TSL2561", "enable", 5000);

        portController.addEventListener(new PortController.PortEventListener() {
            @Override
            public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
                int illuminanceLux = Integer.valueOf(arguments.get(0));

                reportIlluminance(illuminanceLux);
            }
        });

        portController.listenAnalogValueChange(0.01f, 5000, new PortController.PortEventListener() {

            @Override
            public void onPortAnalogValueChange(int id, float value) {
                reportIlluminance(value);
            }
        });
    }

}