package com.stagnationlab.c8y.driver.controllers;

import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioMultiDacActuator;
import com.stagnationlab.c8y.driver.operations.SetChannelValue;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.Util;
import com.stagnationlab.etherio.Commander;

import c8y.lx.driver.OperationExecutor;

public class LightingController extends AbstractController {

	private static final Logger log = LoggerFactory.getLogger(LightingController.class);

	private final com.stagnationlab.c8y.driver.fragments.LightingController lightingController = new com.stagnationlab.c8y.driver.fragments.LightingController();
	private final Map<String, AbstractMultiDacActuator> driverMap = new HashMap<>();

	public LightingController(String id, Map<String, Commander> commanders, Config config) {
		super(id, commanders, config);
	}

	@Override
	protected String getType() {
		return lightingController.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return lightingController;
	}

	@Override
	protected void setup() throws Exception {
		setupLedDrivers();
		setupOperations();
	}

	private void setupOperations() {
		setupSetChannelValueOperation();
	}

	private void setupSetChannelValueOperation() {
		registerOperationExecutor(new OperationExecutor() {
			@Override
			public String supportedOperationType() {
				return Util.buildOperationName(SetChannelValue.class);
			}

			@Override
			public void execute(OperationRepresentation operation, boolean cleanup) throws Exception {
				if (!device.getId().equals(operation.getDeviceId())) {
					return;
				}

				if (cleanup) {
					log.info("ignoring cleanup operation");

					operation.setStatus(OperationStatus.FAILED.toString());

					return;
				}

				SetChannelValue action = operation.get(SetChannelValue.class);

				if (action == null) {
					log.warn("operation is missing the SetChannelValue object");

					return;
				}

				log.info("got operation request to set channel {} to {}", action.getChannel(), action.getValue());

				setLightLevel(action.getChannel(), action.getValue());

				operation.setStatus(OperationStatus.SUCCESSFUL.toString());
			}
		});
	}

	private void setupLedDrivers() {
		int lightCount = config.getInt("lighting.lightCount");

		log.debug("setting up light drivers for {} lights", lightCount);

		for (int lightNumber = 1; lightNumber <= lightCount; lightNumber++) {
			String commanderName = config.getString("lighting.light." + lightNumber + ".commander");

			int ledDriverPort = config.getInt("lighting.driver." + commanderName + ".port");
			int ledDriverChannels = config.getInt("lighting.driver." + commanderName + ".channels");

			Commander commander = commanders.get(commanderName);

			if (!driverMap.containsKey(commanderName)) {
				log.debug("creating light driver for commander {} on port {} for {} channels", commanderName, ledDriverPort, ledDriverChannels);

				AbstractMultiDacActuator driver = new EtherioMultiDacActuator("Led driver for commander " + commanderName, commander, ledDriverPort, ledDriverChannels);

				driverMap.put(commanderName, driver);

				registerChild(driver);
			}
		}
	}

	@Override
	public void start() {
		super.start();

		log.info("starting lighting controller");

		int lightCount = config.getInt("lighting.lightCount");

		for (int lightNumber = 1; lightNumber <= lightCount; lightNumber++) {
			setLightLevel(lightNumber, 1.0f);
		}
	}

	private void setLightLevel(int lightNumber, float value) {
		String commanderName = config.getString("lighting.light." + lightNumber + ".commander");
		int channelIndex = config.getInt("lighting.light." + lightNumber + ".channel");
		AbstractMultiDacActuator driver = driverMap.get(commanderName);

		log.debug("setting light {} to {} on commander {} channel {}", lightNumber, value, commanderName, channelIndex);

		driver.setChannelValue(channelIndex, value);
	}
}
