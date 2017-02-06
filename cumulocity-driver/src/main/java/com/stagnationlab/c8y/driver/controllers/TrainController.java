package com.stagnationlab.c8y.driver.controllers;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.sound.sampled.Port;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class TrainController extends AbstractController {

	class TrainStop {
		private final PortController portController;
		private final String name;
		private static final String TAG_READER_CAPABILITY = "PN532";
		private static final String ENTITY_TRAIN = "TRAIN";
		private static final String ACTION_ENTER = "enter";
		private static final String ACTION_EXIT = "exit";

		TrainStop(PortController portController, String name) {
			this.portController = portController;
			this.name = name;
		}

		void start() {
			portController.sendPortCommand(TAG_READER_CAPABILITY, "enable");

			portController.addEventListener(new PortController.PortEventListener() {
				@Override
				public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
					if (!capabilityName.equals(TAG_READER_CAPABILITY)) {
						return;
					}

					String action = arguments.get(0);
					String entity = arguments.get(1);

					if (!entity.equals(ENTITY_TRAIN)) {
						log.debug("tag reader event {} triggered on {} but expected {}", action, entity, ENTITY_TRAIN);

						return;
					}

					switch (action) {
						case ACTION_ENTER:
							handleTrainEnterEvent();
							break;

						case ACTION_EXIT:
							handleTrainExitEvent();
							break;
					}
				}
			});
		}

		private void handleTrainEnterEvent() {
			log.info("train entered '{}'", name);
		}

		private void handleTrainExitEvent() {
			log.info("train exited '{}'", name);
		}
	}

	private static final Logger log = LoggerFactory.getLogger(LightingController.class);

	private final com.stagnationlab.c8y.driver.fragments.TrainController state = new com.stagnationlab.c8y.driver.fragments.TrainController();
	private Map<Integer, TrainStop> stopMap = new HashMap<>();

	public TrainController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
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
		setupStops();
	}

	@Override
	public void handleEvent(String name, Object info) {

	}

	@Override
	public void start() {
		super.start();

		log.info("starting train controller");

		for (int stopNumber : stopMap.keySet()) {
			TrainStop stop = stopMap.get(stopNumber);

			stop.start();
		}
	}

	private void setupStops() {
		int stopCount = config.getInt("train.stopCount");

		log.debug("configuring {} stops", stopCount);

		for (int i = 0; i < stopCount; i++) {
			setupStop(i);
		}
	}

	private void setupStop(int stopNumber) {
		String commanderName = config.getString("train.stop." + stopNumber + ".commander");
		int port = config.getInt("train.stop." + stopNumber + ".port");
		String name = config.getString("train.stop." + stopNumber + ".name");

		Commander commander = commanders.get(commanderName);
		TrainStop trainStop = createStop(commander, port, name);

		registerStop(stopNumber, trainStop);
	}

	private void registerStop(int stopNumber, TrainStop trainStop) {
		log.info("registering train stop #{} '{}'", stopNumber, trainStop.name);

		stopMap.put(stopNumber, trainStop);
	}

	private TrainStop createStop(Commander commander, int port, String name) {
		PortController portController = new PortController(port, commander);

		return new TrainStop(portController, name);
	}

}
