package com.stagnationlab.c8y.driver.controllers;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.devices.AbstractMotionSensor;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioMotionSensor;
import com.stagnationlab.c8y.driver.fragments.controllers.Motion;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.etherio.Commander;

@Slf4j
public class MotionController extends AbstractController {

	private final Motion state = new Motion();
	private Map<String, AbstractMotionSensor> motionSensorMap = new HashMap<>();

	public MotionController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
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
		log.info("setting up motion controller");

		setupMotionSensors();
	}

	private void setupMotionSensors() {
		List<String> motionSensorNames = config.getStringArray("motion.names");

		for (String motionSensorName : motionSensorNames) {
			setupMotionSensor(motionSensorName);
		}
	}

	private void setupMotionSensor(String name) {
		String commanderName = config.getString("motion." + name + ".commander");
		int portNumber = config.getInt("motion." + name + ".port");

		log.debug("adding motion sensor '{}' on commander {} port {}", name, commanderName, portNumber);

		Commander commander = getCommanderByName(commanderName);
		AbstractMotionSensor motionSensor = new EtherioMotionSensor("Motion sensor: " + name, commander, portNumber);

		motionSensorMap.put(name, motionSensor);

		registerChild(motionSensor);
	}

	@Override
	public void start() {
		super.start();

		log.info("starting motion controller");

		state.setIsRunning(true);
		updateState(state);
	}

	@Override
	public void shutdown() {
		log.info("shutting down motion controller");

		state.reset();
		updateState(state);

		super.shutdown();
	}

}
