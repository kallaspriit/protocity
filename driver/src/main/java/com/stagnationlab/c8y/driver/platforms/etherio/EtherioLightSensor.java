package com.stagnationlab.c8y.driver.platforms.etherio;

import java.util.List;

import com.stagnationlab.c8y.driver.devices.AbstractLightSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class EtherioLightSensor extends AbstractLightSensor {

    private final Commander commander;
    private final int portNumber;
    private PortController portController;
    private int lastReportedIlluminance = -1;

    private static final String CAPABILITY = "TSL2561";

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

        portController.sendPortCommand(CAPABILITY, "enable", 5000);

        portController.addEventListener(new PortController.PortEventListener() {
            @Override
            public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
	            if (!capabilityName.equals(CAPABILITY)) {
		            return;
	            }

                int illuminance = Integer.valueOf(arguments.get(0));

	            // don't report the value if it has not changed
                if (lastReportedIlluminance != -1 && illuminance == lastReportedIlluminance) {
                    return;
                }

                reportIlluminance(illuminance);

                lastReportedIlluminance = illuminance;
            }
        });
    }

}