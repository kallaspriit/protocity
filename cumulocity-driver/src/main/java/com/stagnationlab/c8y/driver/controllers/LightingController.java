package com.stagnationlab.c8y.driver.controllers;

import java.util.HashMap;
import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.stagnationlab.c8y.driver.constants.ControllerEvent;
import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioMultiDacActuator;
import com.stagnationlab.c8y.driver.events.LightingControllerActivatedEvent;
import com.stagnationlab.c8y.driver.fragments.controllers.Lighting;
import com.stagnationlab.c8y.driver.operations.SetAllChannelsValue;
import com.stagnationlab.c8y.driver.operations.SetChannelValue;
import com.stagnationlab.c8y.driver.operations.SetChannelValues;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.c8y.driver.services.Util;
import com.stagnationlab.etherio.Commander;

import c8y.lx.driver.OperationExecutor;

@Slf4j
public class LightingController extends AbstractController {

	private static final float LIGHT_LEVEL_CHANGE_THRESHOLD = 0.01f;
	private static final float LIGHT_LEVEL_TURN_ON_THRESHOLD = 0.2f;
	private static final float LIGHT_LEVEL_TURN_OFF_THRESHOLD = 0.1f;

	private final Lighting state = new Lighting();
	private final Map<String, AbstractMultiDacActuator> driverMap = new HashMap<>();
	private float lastAutomaticLightLevel = 0.0f;
	private final float lowLightValue;
	private final float highLightValue;
	private final float lowLightOutput;
	private final float highLightOutput;

	public LightingController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
		super(id, commanders, config, eventBroker);

		// initialize calibration
		lowLightValue = config.getFloat("lighting.lowLightValue");
		highLightValue = config.getFloat("lighting.highLightValue");
		lowLightOutput = config.getFloat("lighting.lowLightOutput");
		highLightOutput = config.getFloat("lighting.highLightOutput");
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
		log.info("setting up lighting controller");

		setupLedDrivers();
		setupOperations();
	}

	@Override
	public void start() {
		super.start();

		log.info("starting lighting controller");

		state.setIsRunning(true);
		updateState(state);
	}

	@Override
	public void shutdown() {
		log.info("shutting down lighting controller");

		state.reset();
		updateState(state);

		super.shutdown();
	}

	@Override
	public void handleEvent(String name, Object info) {
		if (!state.getIsRunning()) {
			log.debug("got event '{}' but light controller is not running, ignoring it", name);

			return;
		}

		log.trace("got event '{}'", name);

		switch (name) {
			case ControllerEvent.LIGHTMETER_CHANGE:
				handleLightmeterChangeEvent((float)info);
				break;
		}
	}

	private void handleLightmeterChangeEvent(float detectedLightLevel) {
		float outputLightLevel = mapDetectedLightToOutputLevel(detectedLightLevel);
		boolean areLightsCurrentlyOn = lastAutomaticLightLevel > 0.0f;
		boolean forceUpdate = false;

		// avoid flickering the light on and off, require a certain threshold for turning it on
		if (
				(!areLightsCurrentlyOn && outputLightLevel < LIGHT_LEVEL_TURN_ON_THRESHOLD)
				|| (areLightsCurrentlyOn && outputLightLevel <= LIGHT_LEVEL_TURN_OFF_THRESHOLD)
		) {
			outputLightLevel = 0.0f;

			if (outputLightLevel != lastAutomaticLightLevel) {
				log.debug("forcing light output to {}", outputLightLevel);

				forceUpdate = true;
			}
		}

		boolean isChangeSignificant = Math.abs(outputLightLevel - lastAutomaticLightLevel) >= LIGHT_LEVEL_CHANGE_THRESHOLD;

		log.trace("light level change from {} to {} is {}, the update is {}", lastAutomaticLightLevel, outputLightLevel, isChangeSignificant ? "significant" : "not significant", forceUpdate ? "forced" : "not forced");

		// only change the output level if the change is larger than some threshold or update is forced
		if (forceUpdate || isChangeSignificant) {
			log.debug("reacting to detected light level change to {} by setting output power to {}", detectedLightLevel, outputLightLevel);

			setAllLightLevels(outputLightLevel);

			if (lastAutomaticLightLevel == 0.0f && outputLightLevel > 0.0f) {
				handleLightsTurnedOn();
			} else if (lastAutomaticLightLevel > 0.0f && outputLightLevel == 0.0f) {
				handleLightsTurnedOff();
			}

			lastAutomaticLightLevel = outputLightLevel;

			state.setDetectedLightLevel(detectedLightLevel);
			state.setOutputLightLevel(outputLightLevel);

			updateState(state);
		}
	}

	private void handleLightsTurnedOn() {
		log.debug("lights turned on, reporting event");

		reportEvent(new LightingControllerActivatedEvent());
	}

	private void handleLightsTurnedOff() {
		log.debug("lights turned off");
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

				AbstractMultiDacActuator driver = new EtherioMultiDacActuator("Lighting controller led driver for commander " + commanderName, commander, ledDriverPort, ledDriverChannels);

				driverMap.put(commanderName, driver);

				registerChild(driver);
			}
		}
	}

	private void setupOperations() {
		log.debug("setting up operations");

		setupSetChannelValueOperation();
		setupSetChannelValuesOperation();
		setupSetAllChannelsValueOperation();
	}

	private void setupSetChannelValueOperation() {
		log.debug("setting up set channel value operation");

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
		log.debug("setting up set channel values operation");

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
		log.debug("setting up set all channel values operation");

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

	private float mapDetectedLightToOutputLevel(float detectedLightLevel) {
		return Util.map(detectedLightLevel, lowLightValue, highLightValue, lowLightOutput, highLightOutput);
	}

	private void setAllLightLevels(float value) {
		log.debug("setting all light levels to {}", value);

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

			log.trace("setting light {} to {} on commander {} channel {}", lightNumber, value, commanderName, channelIndex);

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

				log.trace("setting light {} to {} on commander {} channel {}", lightNumber, value, commanderName, channelIndex);

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
