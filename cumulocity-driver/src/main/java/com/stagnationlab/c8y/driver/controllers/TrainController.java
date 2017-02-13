package com.stagnationlab.c8y.driver.controllers;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.c8y.driver.services.TextToSpeech;
import com.stagnationlab.c8y.driver.services.Util;
import com.stagnationlab.etherio.Command;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

interface TrainStopEventListener {
	void onTrainEnter(String stopName);
	void onTrainExit(String stopName);
}

public class TrainController extends AbstractController implements TrainStopEventListener, Runnable {

	class TrainStop {
		private final PortController portController;
		private final String name;
		private final List<TrainStopEventListener> eventListeners = new ArrayList<>();

		private static final String TAG_READER_CAPABILITY = "PN532";
		private static final String ENTITY_TRAIN = "TRAIN";
		private static final String ACTION_ENTER = "enter";
		private static final String ACTION_EXIT = "exit";

		TrainStop(PortController portController, String name) {
			this.portController = portController;
			this.name = name;
		}

		void initialize() {
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

		void addEventListener(TrainStopEventListener eventListener) {
			eventListeners.add(eventListener);
		}

		private void handleTrainEnterEvent() {
			log.debug("train entered '{}'", name);

			eventListeners.forEach((listener) -> listener.onTrainEnter(name));
		}

		private void handleTrainExitEvent() {
			log.debug("train exited '{}'", name);

			eventListeners.forEach((listener) -> listener.onTrainExit(name));
		}

		public String getName() {
			return name;
		}
	}

	class Train {

		private final Commander commander;
		private final int normalSpeed;
		private final int requestBatteryVoltageInterval;

		private static final String COMMAND_SET_SPEED = "set-speed";
		private static final String COMMAND_GET_BATTERY_VOLTAGE = "get-battery-voltage";
		private static final String EVENT_OBSTACLE_DETECTED = "obstacle-detected";
		private static final String EVENT_OBSTACLE_CLEARED = "obstacle-cleared";
		private static final String EVENT_SPEED_CHANGED = "speed-changed";

		Train(Commander commander) {
			this.commander = commander;

			normalSpeed = config.getInt("train.normalSpeed");
			requestBatteryVoltageInterval = config.getInt("train.requestBatteryVoltageInterval");

			log.debug("train normal speed: {}%, battery voltage update interval: {}ms", normalSpeed, requestBatteryVoltageInterval);
		}

		void setSpeed(int speed) {
			log.debug("setting train speed to {}", speed);

			speed = Math.min(Math.max(speed, 0), 100);

			commander.sendCommand(COMMAND_SET_SPEED, speed);
		}

		void requestBatteryVoltage() {
			log.debug("requesting battery voltage");

			commander.sendCommand(COMMAND_GET_BATTERY_VOLTAGE).thenAccept((Commander.CommandResponse result) -> {
				float batteryVoltage = result.response.getFloat(0);
				int chargePercentage = result.response.getInt(1);

				handleBatteryVoltageUpdate(batteryVoltage, chargePercentage);
			});

			log.debug("completed requesting battery voltage");
		}

		private void handleBatteryVoltageUpdate(float batteryVoltage, int chargePercentage) {
			log.debug("battery voltage updated: {} ({}%}", batteryVoltage, chargePercentage);

			state.setBatteryVoltage(batteryVoltage);
			state.setChargePercentage(chargePercentage);

			updateState(state);
		}

		private void handleSpeedUpdate(int realSpeed, int targetSpeed) {
			log.debug("train real speed is now {} (target: {})", realSpeed, targetSpeed);

			state.setRealSpeed(realSpeed);
			state.setTargetSpeed(targetSpeed);

			updateState(state);
		}

		void stop() {
			log.debug("stopping train");

			setSpeed(0);
		}

		void forward() {
			setSpeed(normalSpeed);
		}

		public void start() {
			setupEventListeners();
			setupBatteryVoltageListener();
		}

		private void setupEventListeners() {
			commander.addSpecialCommandListener((Command command) -> {
				log.debug("got train special command: {}", command.name);

				List<String> arguments = command.getArguments();

				for (int i = 0; i < arguments.size(); i++) {
					log.debug("- #{}: {}", i, arguments.get(i));
				}

				switch (command.name) {
					case EVENT_OBSTACLE_DETECTED:
						float obstacleDistance = command.getFloat(0);

						handleObstacleDetectedEvent(obstacleDistance);
						break;

					case EVENT_OBSTACLE_CLEARED:
						handleObstacleClearedEvent();
						break;

					case EVENT_SPEED_CHANGED:
						int realSpeed = command.getInt(0);
						int targetSpeed = command.getInt(1);

						handleSpeedChangedEvent(realSpeed, targetSpeed);
						break;
				}
			});
		}

		private void setupBatteryVoltageListener() {
			new Thread(() -> {
				while (isRunning) {
					requestBatteryVoltage();

					try {
						Thread.sleep(requestBatteryVoltageInterval);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}).start();
		}

		private void handleObstacleDetectedEvent(float obstacleDistance) {
			state.setIsObstacleDetected(true);
			state.setObstacleDistance(obstacleDistance);

			updateState(state);
		}

		private void handleObstacleClearedEvent() {
			state.setIsObstacleDetected(false);

			updateState(state);
		}

		private void handleSpeedChangedEvent(int realSpeed, int targetSpeed) {
			state.setRealSpeed(realSpeed);
			state.setTargetSpeed(targetSpeed);

			updateState(state);
		}
	}

	abstract class TrainOperation implements TrainStopEventListener {

		final Train train;

		TrainOperation(Train train) {
			this.train = train;
		}

		public abstract void start();
		public abstract void reset();
		public abstract String getName();
		public abstract boolean isComplete();

		@SuppressWarnings("unused")
		void step(long dt) {}

		@Override
		public void onTrainEnter(String stopName) {}

		@Override
		public void onTrainExit(String stopName) {}

	}

	class DriveToStopTrainOperation extends TrainOperation {

		private String targetStopName;
		private boolean hasEnteredStop = false;
		private boolean isStarted = false;
		private long stopTime = 0;

		DriveToStopTrainOperation(Train train, String targetStopName) {
			super(train);

			this.targetStopName = targetStopName;
		}

		@Override
		public void start() {
			isStarted = true;

			train.forward();

			TextToSpeech.INSTANCE.speak("Next stop: " + targetStopName, true);
		}

		@Override
		public void reset() {
			hasEnteredStop = false;
			isStarted = false;
			stopTime = 0;
		}

		@Override
		public void onTrainEnter(String stopName) {
			if (!isStarted) {
				return;
			}

			if (stopName.equals(targetStopName)) {
				log.debug("train entered target stop '{}', stopping train", targetStopName);

				hasEnteredStop = true;

				train.stop();

				stopTime = Util.now();
			}
		}

		@Override
		public void onTrainExit(String stopName) {}

		public String getName() {
			return "Drive to stop '" + targetStopName + "'";
		}

		@Override
		public boolean isComplete() {
			return isStarted && hasEnteredStop && Util.since(stopTime) >= 1000;
		}
	}

	class WaitTrainOperation extends TrainOperation {

		private final long duration;
		private boolean isStarted = false;
		private long startTime = 0;

		WaitTrainOperation(Train train, long duration) {
			super(train);

			this.duration = duration;
		}

		@Override
		public void start() {
			isStarted = true;
			startTime = Util.now();
		}

		@Override
		public void reset() {
			isStarted = false;
			startTime = 0;
		}

		@Override
		public String getName() {
			return "Wait for " + duration + "ms";
		}

		@Override
		public boolean isComplete() {
			return isStarted && Util.since(startTime) >= duration;
		}
	}

	private static final Logger log = LoggerFactory.getLogger(TrainController.class);

	private final com.stagnationlab.c8y.driver.fragments.TrainController state = new com.stagnationlab.c8y.driver.fragments.TrainController();
	private final List<TrainOperation> operations = new ArrayList<>();
	private final Map<Integer, TrainStop> stopMap = new HashMap<>();
	private Train train;
	private Thread thread;
	private int currentOperationIndex = 0;
	private boolean isRunning = false;

	public TrainController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
		super(id, commanders, config, eventBroker);

		thread = new Thread(this);
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
		setupTrain();
		setupStops();
	}

	@Override
	public void handleEvent(String name, Object info) {

	}

	@Override
	public void onTrainEnter(String stopName) {
		operations.forEach((trainOperation -> trainOperation.onTrainEnter(stopName)));
	}

	@Override
	public void onTrainExit(String stopName) {
		operations.forEach((trainOperation -> trainOperation.onTrainExit(stopName)));
	}

	@Override
	public void start() {
		super.start();

		log.info("starting train controller");

		for (int stopNumber : stopMap.keySet()) {
			TrainStop stop = stopMap.get(stopNumber);

			stop.initialize();
		}

		TrainOperation firstOperation = getCurrentOperation();

		log.info("starting first train operation: {}", firstOperation.getName());

		isRunning = true;

		firstOperation.start();
		train.start();
		thread.start();
	}

	@Override
	public void run() {
		log.info("starting thread");

		long lastStepTime = Util.now();

		while (isRunning) {
			long currentTime = Util.now();
			long deltaTime = currentTime - lastStepTime;

			step(deltaTime);

			lastStepTime = currentTime;

			try {
				Thread.sleep(16); // around 60 FPS
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}

	private void step(long dt) {
		operations.forEach((operation) -> operation.step(dt));

		TrainOperation currentOperation = getCurrentOperation();

		if (currentOperation.isComplete()) {
			log.debug("completed operation: {}", currentOperation.getName());

			currentOperation.reset();

			currentOperationIndex = (currentOperationIndex + 1) % operations.size();

			TrainOperation nextOperation = getCurrentOperation();

			log.debug("starting next operation: {}", nextOperation.getName());

			nextOperation.start();
		}
	}

	private TrainOperation getCurrentOperation() {
		return operations.get(currentOperationIndex);
	}

	private void setupTrain() {
		String commanderName = config.getString("train.commander");
		Commander commander = getCommanderByName(commanderName);

		train = new Train(commander);
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
		int waitTime = config.getInt("train.stop." + stopNumber + ".waitTime");
		String name = config.getString("train.stop." + stopNumber + ".name");

		Commander commander = getCommanderByName(commanderName);
		TrainStop trainStop = createStop(commander, port, name);

		registerStop(stopNumber, trainStop);

		createTrainStopOperations(train, trainStop, waitTime);
	}

	private void createTrainStopOperations(Train train, TrainStop trainStop, int waitTime) {
		registerOperation(
				new DriveToStopTrainOperation(train, trainStop.getName())
		);

		registerOperation(
				new WaitTrainOperation(train, waitTime)
		);
	}

	private TrainStop createStop(Commander commander, int port, String name) {
		PortController portController = new PortController(port, commander);

		return new TrainStop(portController, name);
	}

	private void registerStop(int stopNumber, TrainStop trainStop) {
		log.info("registering train stop #{} '{}'", stopNumber, trainStop.name);

		trainStop.addEventListener(this);

		stopMap.put(stopNumber, trainStop);
	}

	private void registerOperation(TrainOperation operation) {
		operations.add(operation);
	}



}
