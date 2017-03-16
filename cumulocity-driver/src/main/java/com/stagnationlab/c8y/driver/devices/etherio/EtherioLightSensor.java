package com.stagnationlab.c8y.driver.devices.etherio;

import java.util.List;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.Gateway;
import com.stagnationlab.c8y.driver.devices.AbstractLightSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.PortController;

@Slf4j
public class EtherioLightSensor extends AbstractLightSensor {

    private final Commander commander;
    private final int portNumber;
    private PortController portController;
    private int lastReportedIlluminance = -1;

    private static final String LIGHT_SENSOR_CAPABILITY = "TSL2561";
    private static final String COMMAND_ENABLE = "enable";

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

	    commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
		    @Override
		    public void onOpen(boolean isFirstConnect) {
			    log.debug("connection to light sensor '{}' commander has been {}, enabling it", id, isFirstConnect ? "established" : "re-established");

			    if (isFirstConnect) {
				    addEventListeners();
			    }

			    enableSensor();
		    }

		    @Override
		    public void onClose(boolean isPlanned) {
			    log.debug("light sensor '{}' commander transport has been closed", id);
		    }
	    });
    }

	private void addEventListeners() {
		portController.addEventListener(new PortController.PortEventListener() {
			@Override
			public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
				if (!capabilityName.equals(LIGHT_SENSOR_CAPABILITY)) {
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

	private void enableSensor() {
		portController.sendPortCommand(LIGHT_SENSOR_CAPABILITY, COMMAND_ENABLE, 5000)
				.thenAccept(Gateway::handlePortCommandResponse);
	}

}