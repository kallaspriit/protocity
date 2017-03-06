package com.stagnationlab.c8y.driver.controllers;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.c8y.driver.services.TextToSpeech;
import com.stagnationlab.c8y.driver.services.Util;
import com.stagnationlab.etherio.Command;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.PortController;

interface TrainStopEventListener {
	void onTrainEnter(String stopName);
	void onTrainExit(String stopName);
}

@Slf4j
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
		private static final String EVENT_BATTERY_CHARGE_STATE_CHANGED = "battery-charge-state-changed";
		private static final String EVENT_BATTERY_VOLTAGE_CHANGED = "battery-voltage-changed";

		private boolean areEventListenersAdded = false;
		private Thread batteryMonitorThread = null;

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
			commander.sendCommand(COMMAND_GET_BATTERY_VOLTAGE).thenAccept((Commander.CommandResponse result) -> {
				boolean isCharging = result.response.getInt(0) == 1;
				float batteryVoltage = result.response.getFloat(1);
				int chargePercentage = result.response.getInt(2);

				handleBatteryVoltageUpdate(isCharging, batteryVoltage, chargePercentage);
			});
		}

		private void handleBatteryVoltageUpdate(boolean isCharging, float batteryVoltage, int chargePercentage) {
			log.debug("battery is {} with voltage: {} ({}%}", isCharging ? "charging" : "not charging", batteryVoltage, chargePercentage);

			state.setIsCharging(isCharging);
			state.setBatteryVoltage(batteryVoltage);
			state.setBatteryChargePercentage(chargePercentage);

			updateState(state);
		}

		void stop() {
			log.debug("stopping train");

			setSpeed(0);
		}

		void forward() {
			setSpeed(normalSpeed);
		}

		void start() {
			log.debug("starting the train");

			setupEventListeners();
			setupBatteryMonitor();
		}

		void kill() {
			log.debug("killing the train");

			if (batteryMonitorThread != null) {
				log.debug("interrupting train battery monitor thread");

				batteryMonitorThread.interrupt();
			}
		}

		private void setupEventListeners() {
			// only add the listeners once
			if (areEventListenersAdded) {
				return;
			}

			commander.addRemoteCommandListener((Command command) -> {
				log.trace("got train event: {}", command.name);

				List<String> arguments = command.getArguments();

				for (int i = 0; i < arguments.size(); i++) {
					log.trace("- #{}: {}", i, arguments.get(i));
				}

				switch (command.name) {
					case EVENT_OBSTACLE_DETECTED: {
						float obstacleDistance = command.getFloat(0);

						handleObstacleDetectedEvent(obstacleDistance);
						break;
					}

					case EVENT_OBSTACLE_CLEARED: {
						handleObstacleClearedEvent();
						break;
					}

					case EVENT_SPEED_CHANGED: {
						int realSpeed = command.getInt(0);
						int targetSpeed = command.getInt(1);

						handleSpeedChangedEvent(realSpeed, targetSpeed);
						break;
					}

					case EVENT_BATTERY_CHARGE_STATE_CHANGED: {
						boolean isCharging = command.getInt(0) == 1;
						float batteryVoltage = command.getFloat(1);
						int batteryChargePercentage = command.getInt(2);

						handleBatteryChargeStateChanged(isCharging, batteryVoltage, batteryChargePercentage);
						break;
					}

					case EVENT_BATTERY_VOLTAGE_CHANGED: {
						float batteryVoltage = command.getFloat(0);
						int batteryChargePercentage = command.getInt(1);

						handleBatteryVoltageChanged(batteryVoltage, batteryChargePercentage);
						break;
					}

					default:
						log.warn("train event '{}' is not handled", command.name);
						break;
				}
			});

			areEventListenersAdded = true;
		}

		private void setupBatteryMonitor() {
			batteryMonitorThread = new Thread(() -> {
				while (isRunning) {
					requestBatteryVoltage();

					try {
						Thread.sleep(requestBatteryVoltageInterval);
					} catch (InterruptedException e) {
						log.trace("battery monitor sleep was interrupted");
					}
				}
			}, "BatteryMonitor");

			batteryMonitorThread.start();
		}

		private void handleBatteryChargeStateChanged(boolean isCharging, float batteryVoltage, int batteryChargePercentage) {
			log.debug("train is now {}, battery voltage: {}V ({})", isCharging ? "charging" : "not charging", batteryVoltage, batteryChargePercentage);

			state.setIsCharging(isCharging);
			state.setBatteryVoltage(batteryVoltage);
			state.setBatteryChargePercentage(batteryChargePercentage);

			updateState(state);
		}

		private void handleBatteryVoltageChanged(float batteryVoltage, int batteryChargePercentage) {
			log.debug("train battery voltage is now {}V ({})", batteryVoltage, batteryChargePercentage);

			state.setBatteryVoltage(batteryVoltage);
			state.setBatteryChargePercentage(batteryChargePercentage);

			updateState(state);
		}

		private void handleObstacleDetectedEvent(float obstacleDistance) {
			log.debug("train obstacle was detected at {}cm", obstacleDistance);

			state.setIsObstacleDetected(true);
			state.setObstacleDistance(obstacleDistance);

			updateState(state);
		}

		private void handleObstacleClearedEvent() {
			log.debug("train obstacle was cleared");

			state.setIsObstacleDetected(false);

			updateState(state);
		}

		private void handleSpeedChangedEvent(int realSpeed, int targetSpeed) {
			log.debug("train real speed is now {} (target: {})", realSpeed, targetSpeed);

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

		@SuppressWarnings({ "unused", "EmptyMethod" })
		void step(long dt) {
			// nothing by default
		}

		@Override
		public void onTrainEnter(String stopName) {
			// nothing to do
		}

		@SuppressWarnings("EmptyMethod")
		@Override
		public void onTrainExit(String stopName) {
			// nothing to do
		}

	}

	class DriveToStopTrainOperation extends TrainOperation {

		private final String targetStopName;
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

			reportNextStationName(targetStopName);
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

				reportStoppedInStation();
			}
		}

		@Override
		public void onTrainExit(String stopName) {
		}

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

	private final com.stagnationlab.c8y.driver.fragments.controllers.Train state = new com.stagnationlab.c8y.driver.fragments.controllers.Train();
	private final List<TrainOperation> operations = new ArrayList<>();
	private final Map<Integer, TrainStop> stopMap = new HashMap<>();
	private Train train;
	private Thread thread;
	private int currentOperationIndex = 0;
	private boolean isRunning = false;

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
		setupTrain();
		setupStops();
	}

	@SuppressWarnings("EmptyMethod")
	@Override
	public void handleEvent(String name, Object info) {
		// nothing to do
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

		reportConnectionStatus();

		train.commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean wasReconnected) {
				log.info("connection to train commander transport has been {}", wasReconnected ? "re-established" : "opened");

				boolean isFirstConnect = !wasReconnected;

				// only perform some operations on first connect
				if (isFirstConnect) {
					log.debug("initializing train stops");

					for (int stopNumber : stopMap.keySet()) {
						TrainStop stop = stopMap.get(stopNumber);

						stop.initialize();
					}
				}

				TrainOperation trainOperation = getCurrentOperation();

				log.info("starting post-connect train operation: {}", trainOperation.getName());

				isRunning = true;

				trainOperation.start();

				train.start();

				thread = new Thread(TrainController.this, "TrainController");
				thread.start();

				reportOperations();
				reportConnectionStatus();
			}

			@Override
			public void onClose() {
				log.info("train commander transport has been closed");

				TextToSpeech.INSTANCE.speak("Wireless connection to the train was lost, attempting to reestablish");

				train.kill();

				isRunning = false;

				if (thread != null) {
					try {
						thread.join();
						thread = null;
					} catch (InterruptedException e) {
						log.warn("waiting for train thread to join failed ({} - {})", e.getClass().getSimpleName(), e.getMessage(), e);
					}
				}

				log.debug("stopped train controller");

				reportConnectionStatus();
			}
		});
	}

	@Override
	public void run() {
		log.info("starting train update thread");

		long lastStepTime = Util.now();

		while (isRunning) {
			long currentTime = Util.now();
			long deltaTime = currentTime - lastStepTime;

			step(deltaTime);

			lastStepTime = currentTime;

			try {
				Thread.sleep(16); // around 60 FPS
			} catch (InterruptedException e) {
				log.debug("train thread was interrupted");
			}
		}

		log.debug("stopped train update thread");
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
			reportCurrentOperationIndex();
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

	private void reportCurrentOperationIndex() {
		state.setCurrentOperationIndex(currentOperationIndex);

		updateState(state);
	}

	private void reportConnectionStatus() {
		state.setIsConnected(isRunning);

		updateState(state);
	}

	private void reportOperations() {
		List<String> operationsNames = operations
				.stream()
				.map(TrainOperation::getName)
				.collect(Collectors.toList());

		state.setOperations(operationsNames);
		state.setCurrentOperationIndex(currentOperationIndex);

		updateState(state);
	}

	private void reportNextStationName(String nextStationName) {
		String currentStopName = state.getNextStationName();

		state.setPreviousStationName(currentStopName);
		state.setNextStationName(nextStationName);
		state.setIsInStation(false);

		updateState(state);
	}

	private void reportStoppedInStation() {
		state.setIsInStation(true);

		updateState(state);
	}

}
