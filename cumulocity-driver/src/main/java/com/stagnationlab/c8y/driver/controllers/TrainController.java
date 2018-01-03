package com.stagnationlab.c8y.driver.controllers;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.Gateway;
import com.stagnationlab.c8y.driver.events.ControllerActivatedEvent;
import com.stagnationlab.c8y.driver.events.ControllerDeactivatedEvent;
import com.stagnationlab.c8y.driver.measurements.BatteryMeasurement;
import com.stagnationlab.c8y.driver.services.BatteryMonitor;
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

	private static final String TAG_READER_CAPABILITY = "PN532";
	private static final String COMMAND_ENABLE = "enable";
	private static final String ACTION_ENTER = "enter";
	private static final String ACTION_EXIT = "exit";
	private static final String ACTION_UID = "uid";
	private static final long MINIMUM_TICKET_BUYING_TIMEOUT = 5000;

	class TrainStop {
		private final PortController portController;
		private final String name;
		private final List<TrainStopEventListener> eventListeners = new ArrayList<>();
		private final List<String> trainTagUidList;

		TrainStop(PortController portController, String name, List<String> trainTagUidList) {
			this.portController = portController;
			this.name = name;
			this.trainTagUidList = trainTagUidList;
		}

		void addEventListener(TrainStopEventListener eventListener) {
			eventListeners.add(eventListener);
		}

		void initialize() {
			portController.getCommander().getMessageTransport().addEventListener(new MessageTransport.EventListener() {

				@Override
				public void onOpen(boolean isFirstConnect) {
					if (isFirstConnect) {
						setupEventListeners();
					}

					startTagListener();
				}
			});
		}

		private void setupEventListeners() {
			portController.addEventListener(new PortController.PortEventListener() {
				@Override
				public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
					if (!capabilityName.equals(TAG_READER_CAPABILITY)) {
						return;
					}

					String type = arguments.get(0);

					switch (type) {
						case ACTION_UID: {
							String action = arguments.get(1);
							String uid = arguments.get(2);

							handleTagEvent(action, uid);
							break;
						}
					}
				}
			});
		}

		private void startTagListener() {
			// request the NFC capability on transport open
			portController.sendPortCommand(TAG_READER_CAPABILITY, COMMAND_ENABLE)
				.thenAccept(Gateway::handlePortCommandResponse);
		}

		private void handleTagEvent(String action, String uid) {
			log.debug("got tag '{}' for '{}'", action, uid);

			switch (action) {
				case ACTION_ENTER:
					handleTagEnter(uid);
					break;

				case ACTION_EXIT:
					handleTagExit(uid);
					break;
			}
		}

		private void handleTagEnter(String uid) {
			if (!trainTagUidList.contains(uid)) {
				return;
			}

			log.debug("train tag '{}' entered", uid);

			eventListeners.forEach((listener) -> listener.onTrainEnter(name));
		}

		private void handleTagExit(String uid) {
			if (!trainTagUidList.contains(uid)) {
				return;
			}

			log.debug("train tag '{}' exited", uid);

			eventListeners.forEach((listener) -> listener.onTrainExit(name));
		}

		public String getName() {
			return name;
		}
	}

	class Train {

		private static final int MIN_OBSTACLE_DETECTED_REPORT_PAUSE_MS = 3000;

		private final Commander commander;
		private final BatteryMonitor batteryMonitor;
		private final int normalSpeed;
		private final int requestBatteryVoltageInterval;

		private static final String COMMAND_GET_BATTERY_VOLTAGE = "battery";
		private static final String SET_OBSTACLE_PARAMETERS = "set-obstacle-parameters";
		private static final String COMMAND_SET_SPEED = "set-speed";
		private static final String EVENT_OBSTACLE_DETECTED = "obstacle-detected";
		private static final String EVENT_OBSTACLE_CLEARED = "obstacle-cleared";
		private static final String EVENT_OBSTACLE_CHANGED = "obstacle-changed";
		private static final String EVENT_SPEED_CHANGED = "speed-changed";
		private static final String EVENT_BATTERY_STATE_CHANGED = "battery-state-changed";

		private boolean areEventListenersAdded = false;
		private boolean isExpectingObstacleClearedEvent = false;
		private long lastObstacleNotificationTime = 0;

		Train(Commander commander) {
			this.commander = commander;

			normalSpeed = config.getInt("train.normalSpeed");
			requestBatteryVoltageInterval = config.getInt("train.requestBatteryVoltageInterval");

			int lowBatteryPercentageThreshold = config.getInt("train.lowBatteryPercentageThreshold");
			int lowBatteryReportingInterval = config.getInt("train.lowBatteryReportingInterval");

			batteryMonitor = new BatteryMonitor("The train", lowBatteryPercentageThreshold, lowBatteryReportingInterval);

			log.debug("train normal speed: {}%, battery voltage update interval: {}ms", normalSpeed, requestBatteryVoltageInterval);
		}

		void setSpeed(int speed) {
			log.debug("setting train speed to {}", speed);

			speed = Math.min(Math.max(speed, 0), 100);

			commander.sendCommand(COMMAND_SET_SPEED, speed);
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
			requestBatteryVoltage();
			setObstacleParameters();
		}

		void shutdown() {
			log.debug("shutting down the train");
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

					case EVENT_OBSTACLE_CHANGED: {
						float obstacleDistance = command.getFloat(0);

						handleObstacleChangedEvent(obstacleDistance);
						break;
					}

					case EVENT_SPEED_CHANGED: {
						int realSpeed = command.getInt(0);
						int targetSpeed = command.getInt(1);

						handleSpeedChangedEvent(realSpeed, targetSpeed);
						break;
					}

					case EVENT_BATTERY_STATE_CHANGED: {
						boolean isCharging = command.getInt(0) == 1;
						float batteryVoltage = command.getFloat(1);
						int batteryChargePercentage = command.getInt(2);

						handleBatteryChargeStateChanged(isCharging, batteryVoltage, batteryChargePercentage);
						break;
					}

					default:
						log.warn("train event '{}' is not handled", command.name);
						break;
				}
			});

			areEventListenersAdded = true;
		}

		private void handleObstacleChangedEvent(float obstacleDistance) {
			// only update the state if an obstacle is detected
			if (state.getIsObstacleDetected()) {
				log.debug("train obstacle distance changed to {} cm", obstacleDistance);

				state.setObstacleDistance(obstacleDistance);

				updateState(state);
			}
		}

		private void requestBatteryVoltage() {
			commander.sendCommand(COMMAND_GET_BATTERY_VOLTAGE).thenAccept((Commander.CommandResponse result) -> {
				boolean isCharging = result.response.getInt(0) == 1;
				float batteryVoltage = result.response.getFloat(1);
				int batteryChargePercentage = result.response.getInt(2);

				handleBatteryChargeStateChanged(isCharging, batteryVoltage, batteryChargePercentage);
			});
		}

		private void setObstacleParameters() {
			float obstacleDetectedThreshold = config.getFloat("train.obstacleDetectedThreshold");
			float obstacleClearedThreshold = config.getFloat("train.obstacleClearedThreshold");
			int brakeDuration = config.getInt("train.brakeDuration");

			log.debug("setting detected obstacle distance at {}cm and cleared at {}cm", obstacleDetectedThreshold, obstacleClearedThreshold);

			commander.sendCommand(
					SET_OBSTACLE_PARAMETERS,
					obstacleDetectedThreshold,
					obstacleClearedThreshold,
					brakeDuration
			);
		}

		private void handleBatteryChargeStateChanged(boolean isCharging, float batteryVoltage, int batteryChargePercentage) {
			log.debug("train is {}, battery voltage: {}V ({}%)", isCharging ? "charging" : "not charging", batteryVoltage, batteryChargePercentage);

			state.setIsCharging(isCharging);
			state.setBatteryVoltage(batteryVoltage);
			state.setBatteryChargePercentage(batteryChargePercentage);

			updateState(state);
			reportMeasurement(new BatteryMeasurement(batteryVoltage, batteryChargePercentage, isCharging));

			batteryMonitor.checkForLowBattery(batteryChargePercentage, isCharging);
		}

		private void handleObstacleDetectedEvent(float obstacleDistance) {
			log.debug("train obstacle was detected at {}cm", obstacleDistance);

			state.setIsObstacleDetected(true);
			state.setObstacleDistance(obstacleDistance);

			updateState(state);

			if (state.getTargetSpeed() > 0 && Util.since(lastObstacleNotificationTime) >= MIN_OBSTACLE_DETECTED_REPORT_PAUSE_MS) {
				TextToSpeech.INSTANCE.speak("Obstacle detected", true);

				lastObstacleNotificationTime = Util.now();
				isExpectingObstacleClearedEvent = true;
			}
		}

		private void handleObstacleClearedEvent() {
			log.debug("train obstacle was cleared");

			state.setIsObstacleDetected(false);

			updateState(state);

			if (isExpectingObstacleClearedEvent && state.getTargetSpeed() > 0) {
				TextToSpeech.INSTANCE.speak("Resuming", true);

				isExpectingObstacleClearedEvent = false;
			}
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
		private final boolean isFinalStop;
		private boolean hasEnteredStop = false;
		private boolean isStarted = false;
		private long stopTime = 0;

		DriveToStopTrainOperation(Train train, String targetStopName, boolean isFinalStop) {
			super(train);

			this.targetStopName = targetStopName;
			this.isFinalStop = isFinalStop;
		}

		@Override
		public String getName() {
			return "DRIVE:" + targetStopName;
		}

		@Override
		public void start() {
			isStarted = true;

			train.forward();

			if (isFinalStop) {
				TextToSpeech.INSTANCE.speak("Final stop: " + targetStopName, false);
			} else {
				TextToSpeech.INSTANCE.speak("Next stop: " + targetStopName, false);
			}

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

		@Override
		public boolean isComplete() {
			return isStarted && hasEnteredStop && Util.since(stopTime) >= 1000;
		}

		String getTargetStopName() {
			return targetStopName;
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
			return "WAIT:" + duration;
		}

		@Override
		public boolean isComplete() {
			return isStarted && Util.since(startTime) >= duration;
		}
	}

	class IdleTrainOperation extends TrainOperation {

		private long startTime = 0;

		IdleTrainOperation(Train train) {
			super(train);
		}

		@Override
		public void start() {
			log.debug("reporting controller deactivated event");

			reportEvent(new ControllerDeactivatedEvent());

			startTime = System.currentTimeMillis();
		}

		@Override
		public void reset() {
		}

		@Override
		public String getName() {
			return "IDLE";
		}

		@Override
		public boolean isComplete() {
			long currentTime = System.currentTimeMillis();
			long idleDuration = currentTime - startTime;
			long notChargingThreshold = 10 * 1000;
			boolean isIdleForLong = idleDuration >= notChargingThreshold;

			// exit idle state if not charging and has been idle for a while
			if (!state.getIsCharging() && isIdleForLong) {
				TextToSpeech.INSTANCE.speak("Going for a lap", true);

				return true;
			}

			return false;
		}
	}

	class TripCompleteTrainOperation extends TrainOperation {

		TripCompleteTrainOperation(Train train) {
			super(train);
		}

		@Override
		public void start() {
			TextToSpeech.INSTANCE.speak("Trip complete, the ticket price was deducted rom your mobile wallet", true);
		}

		@Override
		public void reset() {
		}

		@Override
		public String getName() {
			return "TRIP-COMPLETE";
		}

		@Override
		public boolean isComplete() {
			return true;
		}
	}

	private com.stagnationlab.c8y.driver.fragments.controllers.Train state = new com.stagnationlab.c8y.driver.fragments.controllers.Train();
	private final List<TrainOperation> operations = new ArrayList<>();
	private final Map<Integer, TrainStop> stopMap = new HashMap<>();
	private Train train;
	private Thread thread;
	private int currentOperationIndex = 0;
	private long lastTicketActivationTime = 0;

	private PortController ticketController;

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
		log.info("setting up train controller");

		setupTrain();
		setupStartOperations();
		setupStops();
		setupTicketTerminal();
		setupEndOperations();
	}

	@Override
	public void start() {
		super.start();

		log.info("starting train controller");

		restoreExistingState();

		train.commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to train commander has been {}", isFirstConnect ? "established" : "re-established");

				// only perform some operations on first connect
				if (isFirstConnect) {
					log.debug("initializing train stops");

					for (int stopNumber : stopMap.keySet()) {
						TrainStop stop = stopMap.get(stopNumber);

						stop.initialize();
					}
				}

				TrainOperation trainOperation = getCurrentOperation();

				log.debug("starting post-connect train operation: {}", trainOperation.getName());

				setIsRunning(true);

				trainOperation.start();

				train.start();

				thread = new Thread(TrainController.this, "TrainController");
				thread.start();

				reportOperations();

				if (!isFirstConnect) {
					TextToSpeech.INSTANCE.speak("5G connection to the train has been reestablished", false);
				}
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.info("train commander transport has been closed");

				setIsRunning(false);

				train.shutdown();

				if (thread != null) {
					try {
						thread.join();
						thread = null;
					} catch (InterruptedException e) {
						log.warn("waiting for train thread to join failed ({} - {})", e.getClass().getSimpleName(), e.getMessage(), e);
					}
				}

				log.debug("stopped train controller");

				if (!isPlanned) {
					TextToSpeech.INSTANCE.speak("Connection to the train was lost", false);
				}
			}
		});
	}

	private void restoreExistingState() {
		log.debug("restoring existing state");

		com.stagnationlab.c8y.driver.fragments.controllers.Train existingState = device.get(com.stagnationlab.c8y.driver.fragments.controllers.Train.class);

		if (existingState != null) {
			state = existingState;

			log.debug("restored existing state");
		} else {
			log.info("no existing state found");
		}
	}

	@Override
	public void shutdown() {
		log.info("shutting down train controller");

		state.reset();
		updateState(state);

		super.shutdown();
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
	public void run() {
		log.info("starting train update thread");

		long lastStepTime = Util.now();

		while (state.getIsRunning()) {
			long currentTime = Util.now();
			long deltaTime = currentTime - lastStepTime;

			step(deltaTime);

			lastStepTime = currentTime;

			//try {
			//	Thread.sleep(16); // around 60 FPS
			//} catch (InterruptedException e) {
			//	log.debug("train thread was interrupted");
			//}
		}

		log.debug("stopped train update thread");
	}

	private void step(long dt) {
		operations.forEach((operation) -> operation.step(dt));

		TrainOperation currentOperation = getCurrentOperation();

		if (currentOperation.isComplete()) {
			startNextOperation();
		}
	}

	private void startNextOperation() {
		TrainOperation currentOperation = getCurrentOperation();

		log.debug("completed operation: {}", currentOperation.getName());

		currentOperation.reset();

		currentOperationIndex = (currentOperationIndex + 1) % operations.size();

		TrainOperation nextOperation = getCurrentOperation();

		log.debug("starting next operation: {}", nextOperation.getName());

		nextOperation.start();
		reportCurrentOperationIndex();
	}

	private TrainOperation getCurrentOperation() {
		return operations.get(currentOperationIndex);
	}

	private TrainOperation getPreviousOperation() {
		int previousOperationIndex = (currentOperationIndex - 1) % operations.size();

		return operations.get(previousOperationIndex);
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
			setupStop(i, stopCount);
		}
	}

	private void setupStop(int stopNumber, int stopCount) {
		String commanderName = config.getString("train.stop." + stopNumber + ".commander");
		int port = config.getInt("train.stop." + stopNumber + ".port");
		int waitTime = config.getInt("train.stop." + stopNumber + ".waitTime");
		String name = config.getString("train.stop." + stopNumber + ".name");
		List<String> trainTagUidList = config.getStringArray("train.tagUidList");

		Commander commander = getCommanderByName(commanderName);
		TrainStop trainStop = createStop(commander, port, name, trainTagUidList);

		registerStop(stopNumber, trainStop);

		boolean isFinalStop = stopNumber == stopCount - 1;

		// stop at given station
		registerOperation(
				new DriveToStopTrainOperation(train, trainStop.getName(), isFinalStop)
		);

		// don't add the stop after the last station
		if (!isFinalStop) {
			registerOperation(
					new WaitTrainOperation(train, waitTime)
			);
		}
	}

	private void setupTicketTerminal() {
		String commanderName = config.getString("train.ticket.commander");
		int port = config.getInt("train.ticket.port");
		List<String> tagUidList = config.getStringArray("train.ticket.tagUidList");

		log.debug("configuring ticket terminal on commander {} port {}", commanderName, port);

		Commander ticketCommander = getCommanderByName(commanderName);
		ticketController = new PortController(port, ticketCommander);

		ticketCommander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("ticket commander transport was opened, enabling tag reader");

				ticketController.sendPortCommand(TAG_READER_CAPABILITY, COMMAND_ENABLE)
						.thenAccept(Gateway::handlePortCommandResponse);

				if (isFirstConnect) {
					ticketController.addEventListener(new PortController.PortEventListener() {
						@Override
						public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
							if (!capabilityName.equals(TAG_READER_CAPABILITY)) {
								return;
							}

							String type = arguments.get(0);

							switch (type) {
								case ACTION_UID: {
									String action = arguments.get(1);
									String uid = arguments.get(2);
									//boolean isValidTag = tagUidList.contains(uid);
									boolean isValidTag = true; // disabled checking tag - any tag will do

									if (action.equals(ACTION_ENTER) && isValidTag) {
										handleTrainTicketBought();
									}

									break;
								}
							}
						}
					});
				}
			}
		});
	}

	private void setupStartOperations() {
		registerOperation(
				new IdleTrainOperation(train)
		);
	}

	private void setupEndOperations() {
		registerOperation(
				new TripCompleteTrainOperation(train)
		);
	}

	private void handleTrainTicketBought() {
		if (Util.since(lastTicketActivationTime) < MINIMUM_TICKET_BUYING_TIMEOUT) {
			log.debug("requested buying ticket too soon, ignoring it");

			return;
		}

		log.debug("train ticket was bought");

		TrainOperation currentOperation = getCurrentOperation();

		// decide what to do based on current train operation
		if (currentOperation instanceof IdleTrainOperation) {
			log.debug("train is currently idle, starting new loop");

			TextToSpeech.INSTANCE.speak("Train ticket has been bought", true);

			state.incrementNumberOfTicketsBought();

			log.debug("incremented the number of tickets bought to {}", state.getNumberOfTicketsBought());

			startNextOperation();

			log.debug("reporting controller activated event");

			reportEvent(new ControllerActivatedEvent());
		} else if (currentOperation instanceof DriveToStopTrainOperation) {
			String targetStopName =((DriveToStopTrainOperation) currentOperation).getTargetStopName();

			log.debug("train is currently en route to {}, wait for it to arrive at central station and become idle", targetStopName);

			TextToSpeech.INSTANCE.speak("Train is currently en-route to " + targetStopName + ", please wait for it to return", true);
		} else if (currentOperation instanceof WaitTrainOperation) {
			String currentStopName = "";

			TrainOperation previousOperation = getPreviousOperation();

			if (previousOperation instanceof DriveToStopTrainOperation) {
				currentStopName = ((DriveToStopTrainOperation) previousOperation).getTargetStopName();
			}

			if (currentStopName.length() > 0) {
				log.debug("train is currently waiting in {}, wait for it to arrive at central station and become idle", currentStopName);

				TextToSpeech.INSTANCE.speak("Train is currently waiting in " + currentStopName + ", please wait for it to return", true);
			} else {
				TextToSpeech.INSTANCE.speak("Please wait for the train to return to Central Station", true);
			}
		}

		lastTicketActivationTime = Util.now();
	}

	private TrainStop createStop(Commander commander, int port, String name, List<String> trainTagUidList) {
		PortController portController = new PortController(port, commander);

		return new TrainStop(portController, name, trainTagUidList);
	}

	private void registerStop(int stopNumber, TrainStop trainStop) {
		log.debug("registering train stop #{} '{}'", stopNumber, trainStop.name);

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

	private void setIsRunning(boolean isRunning) {
		state.setIsRunning(isRunning);

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
