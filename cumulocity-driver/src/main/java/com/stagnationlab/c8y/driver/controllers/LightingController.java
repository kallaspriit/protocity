package com.stagnationlab.c8y.driver.controllers;

import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.stagnationlab.c8y.driver.constants.ControllerEvent;
import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioMultiDacActuator;
import com.stagnationlab.c8y.driver.operations.SetAllChannelsValue;
import com.stagnationlab.c8y.driver.operations.SetChannelValue;
import com.stagnationlab.c8y.driver.operations.SetChannelValues;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.c8y.driver.services.Util;
import com.stagnationlab.etherio.Commander;

import c8y.lx.driver.OperationExecutor;

public class LightingController extends AbstractController {

	private static final Logger log = LoggerFactory.getLogger(LightingController.class);
	public static final float LIGHT_LEVEL_CHANGE_THRESHOLD = 0.01f;
	public static final float LIGHT_LEVEL_TURN_ON_THRESHOLD = 0.2f;
	public static final float LIGHT_LEVEL_TURN_OFF_THRESHOLD = 0.1f;

	private final com.stagnationlab.c8y.driver.fragments.LightingController state = new com.stagnationlab.c8y.driver.fragments.LightingController();
	private final Map<String, AbstractMultiDacActuator> driverMap = new HashMap<>();
	private float lastAutomaticLightLevel = 0.0f;

	public LightingController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
		super(id, commanders, config, eventBroker);
	}

	@Override
	protected String getType() {
		return state.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return state;
	}

	@Override
	protected void setup() throws Exception {
		setupLedDrivers();
		setupOperations();
	}

	@Override
	public void handleEvent(String name, Object info) {
		switch (name) {
			case ControllerEvent.LIGHTMETER_CHANGE:
				handleLightmeterChangeEvent((float)info);
				break;
		}
	}

	@Override
	public void start() {
		super.start();

		log.info("starting lighting controller");
	}

	private void handleLightmeterChangeEvent(float detectedLightLevel) {
		float outputLightLevel = Util.map(detectedLightLevel, 10.0f, 200.0f, 0.5f, 0.0f);
		boolean areLightsCurrentlyOn = lastAutomaticLightLevel > 0.0f;
		boolean forceUpdate = false;

		// avoid flickering the light on and off, require a certain threshold for turning it on
		if (
				(!areLightsCurrentlyOn && outputLightLevel < LIGHT_LEVEL_TURN_ON_THRESHOLD)
				|| (areLightsCurrentlyOn && outputLightLevel <= LIGHT_LEVEL_TURN_OFF_THRESHOLD)
		) {
			outputLightLevel = 0.0f;

			if (outputLightLevel != lastAutomaticLightLevel) {
				forceUpdate = true;
			}
		}
		
		boolean isChangeSignificant = Math.abs(outputLightLevel - lastAutomaticLightLevel) >= LIGHT_LEVEL_CHANGE_THRESHOLD;

		// only change the output level if the change is larger than some threshold or update is forced
		if (forceUpdate || isChangeSignificant) {
			setAllLightLevels(outputLightLevel);

			lastAutomaticLightLevel = outputLightLevel;

			log.debug("lightmeter value changed to {}, setting light level to {}", detectedLightLevel, outputLightLevel);

			state.setDetectedLightLevel(detectedLightLevel);
			state.setOutputLightLevel(outputLightLevel);

			updateState(state);
		}
	}

	private void setupLedDrivers() {
		int lightCount = config.getInt("lighting.lightCount");

		log.debug("setting up light drivers for {} lights", lightCount);

		for (int lightNumber = 0; lightNumber < lightCount; lightNumber++) {
			String commanderName = config.getString("lighting.light." + lightNumber + ".commander");

			int ledDriverPort = config.getInt("lighting.driver." + commanderName + ".port");
			int ledDriverChannels = config.getInt("lighting.driver." + commanderName + ".channels");

			Commander commander = getCommanderByName(commanderName);

			if (!driverMap.containsKey(commanderName)) {
				log.debug("creating light driver for commander {} on port {} for {} channels", commanderName, ledDriverPort, ledDriverChannels);

				AbstractMultiDacActuator driver = new EtherioMultiDacActuator("Led driver for commander " + commanderName, commander, ledDriverPort, ledDriverChannels);

				driverMap.put(commanderName, driver);

				registerChild(driver);
			}
		}
	}

	private void setupOperations() {
		setupSetChannelValueOperation();
		setupSetChannelValuesOperation();
		setupSetAllChannelsValueOperation();
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

	private void setupSetChannelValuesOperation() {
		registerOperationExecutor(new OperationExecutor() {
			@Override
			public String supportedOperationType() {
				return Util.buildOperationName(SetChannelValues.class);
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

				SetChannelValues action = operation.get(SetChannelValues.class);

				if (action == null) {
					log.warn("operation is missing the SetChannelValues object");

					return;
				}

				log.info("got operation request to set multiple channel values");

				Map<String, Number> values = action.getValues();
				Map<Integer, Float> channelValueMap = new HashMap<>();

				for (Map.Entry<String, Number> channelValuePair : values.entrySet()) {
					int lightNumber = Integer.valueOf(channelValuePair.getKey());
					float value = channelValuePair.getValue().floatValue();

					log.debug("- {}: {}", lightNumber, value);

					channelValueMap.put(lightNumber, value);
				}

				setLightLevels(channelValueMap);

				operation.setStatus(OperationStatus.SUCCESSFUL.toString());
			}
		});
	}

	private void setupSetAllChannelsValueOperation() {
		registerOperationExecutor(new OperationExecutor() {
			@Override
			public String supportedOperationType() {
				return Util.buildOperationName(SetAllChannelsValue.class);
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

				SetAllChannelsValue action = operation.get(SetAllChannelsValue.class);

				if (action == null) {
					log.warn("operation is missing the SetAllChannelsValue object");

					return;
				}

				float value = action.getValue();

				log.info("got operation request to set all channels to value: {}", value);

				setAllLightLevels(value);

				operation.setStatus(OperationStatus.SUCCESSFUL.toString());
			}
		});
	}

	private void setAllLightLevels(float value) {
		Map<Integer, Float> channelValueMap = new HashMap<>();
		int lightCount = config.getInt("lighting.lightCount");

		for (int lightNumber = 0; lightNumber < lightCount; lightNumber++) {
			channelValueMap.put(lightNumber, value);
		}

		setLightLevels(channelValueMap);
	}

	private void setLightLevel(int lightNumber, float value) {
		try {
			String commanderName = config.getString("lighting.light." + lightNumber + ".commander");
			int channelIndex = config.getInt("lighting.light." + lightNumber + ".channel");
			AbstractMultiDacActuator driver = driverMap.get(commanderName);

			log.debug("setting light {} to {} on commander {} channel {}", lightNumber, value, commanderName, channelIndex);

			driver.setChannelValue(channelIndex, value);
		} catch (Exception e) {
			log.warn("setting light {} to {} failed ({} - {})", lightNumber, value, e.getClass().getSimpleName(), e.getMessage());
		}
	}

	private void setLightLevels(Map<Integer, Float> channelValueMap) {
		Map<String, Map<Integer, Float>> commanderChannelValueMap = new HashMap<>();

		for (Map.Entry<Integer, Float> channelValuePair : channelValueMap.entrySet()) {
			int lightNumber = channelValuePair.getKey();
			float value = channelValuePair.getValue();

			try {
				String commanderName = config.getString("lighting.light." + lightNumber + ".commander");
				int channelIndex = config.getInt("lighting.light." + lightNumber + ".channel");

				if (!commanderChannelValueMap.containsKey(commanderName)) {
					commanderChannelValueMap.put(commanderName, new HashMap<>());
				}

				commanderChannelValueMap.get(commanderName).put(channelIndex, value);
			} catch (Exception e) {
				log.warn("setting light {} to {} failed ({} - {})", lightNumber, value, e.getClass().getSimpleName(), e.getMessage());
			}
		}

		for (String commanderName : commanderChannelValueMap.keySet()) {
			Map<Integer, Float> values = commanderChannelValueMap.get(commanderName);
			AbstractMultiDacActuator driver = driverMap.get(commanderName);

			driver.setChannelValues(values);
		}
	}
}
