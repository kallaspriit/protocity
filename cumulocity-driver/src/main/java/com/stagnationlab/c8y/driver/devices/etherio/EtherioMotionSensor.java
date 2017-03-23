package com.stagnationlab.c8y.driver.devices.etherio;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.Gateway;
import com.stagnationlab.c8y.driver.devices.AbstractMotionSensor;
import com.stagnationlab.c8y.driver.fragments.sensors.MotionSensor;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.PortController;

@SuppressWarnings("unused")
@Slf4j
public class EtherioMotionSensor extends AbstractMotionSensor {

	private final Commander commander;
	private final int portNumber;
	private PortController portController;
	private static final String RESPONSE_OK = "OK";

	public EtherioMotionSensor(String id, Commander commander, int portNumber) {
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
		log.debug("starting motion sensor '{}'", id);

		commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to tag sensor '{}' commander has been {}, enabling it", id, isFirstConnect ? "established" : "re-established");

				// listen for port analog value changes
				if (isFirstConnect) {
					portController.addEventListener(new PortController.PortEventListener() {
						@Override
						public void onPortDigitalValueChange(int id, PortController.DigitalValue digitalValue) {
							boolean isMotionDetected = digitalValue == PortController.DigitalValue.LOW;
							boolean wasMotionDetected = state.getMotionState() == MotionSensor.MotionState.MOTION_DETECTED;

							// don't report the event if the state has not changed
							if (isMotionDetected == wasMotionDetected) {
								return;
							}

							log.debug("motion for {} {}", EtherioMotionSensor.this.id, isMotionDetected ? "was detected" : "was cleared");

							setIsMotionDetected(isMotionDetected);
						}
					});
				}

				portController.setPortMode(PortController.PortMode.INTERRUPT).thenAccept(commandResponse -> {
					if (commandResponse.response.name.equals(RESPONSE_OK)) {
						state.setIsRunning(true);
						updateState(state);

						portController.setPullMode(PortController.PullMode.UP).thenAccept(Gateway::handlePortCommandResponse);
					}

					Gateway.handlePortCommandResponse(commandResponse);
				});
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.debug("motion sensor '{}' commander transport has been closed", id);

				state.setIsRunning(false);
				updateState(state);
			}
		});
	}

}
