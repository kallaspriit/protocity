package com.stagnationlab.c8y.driver.controllers;

import java.util.List;
import java.util.Map;
import java.util.concurrent.ScheduledFuture;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.devices.AbstractAnalogInputSensor;
import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioAnalogInputSensor;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioMultiDacActuator;
import com.stagnationlab.c8y.driver.events.ControllerActivatedEvent;
import com.stagnationlab.c8y.driver.fragments.controllers.Truck;
import com.stagnationlab.c8y.driver.measurements.BatteryMeasurement;
import com.stagnationlab.c8y.driver.measurements.ChargePowerMeasurement;
import com.stagnationlab.c8y.driver.measurements.GridPowerBalanceMeasurement;
import com.stagnationlab.c8y.driver.services.BatteryMonitor;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.c8y.driver.services.Scheduler;
import com.stagnationlab.c8y.driver.services.SimulatedVariance;
import com.stagnationlab.c8y.driver.services.TextToSpeech;
import com.stagnationlab.c8y.driver.services.Util;
import com.stagnationlab.etherio.Command;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;

@Slf4j
public class TruckController extends AbstractController {

	private static final int CHARGE_POWER_INTERVAL_MS = 10000;
	private static final int CONTROLLER_ACTIVATION_MIN_PAUSE = 30000;

	private final Truck state = new Truck();
	private Commander truckCommander;
	private Commander indicatorCommander;
	private final BatteryMonitor batteryMonitor;
	private AbstractMultiDacActuator indicatorDriver;
	private AbstractAnalogInputSensor solarPanelSensor;
	private SimulatedVariance chargePowerVariance;
	private ScheduledFuture<?> chargePowerInterval;
	private float truckBaseChargePower = 0.0f;
	private int indicatorChannel = 0;
	private long lastActivationReportedTime = 0;

	private static final String COMMAND_GET_BATTERY_VOLTAGE = "battery";
	private static final String EVENT_BATTERY_STATE_CHANGED = "battery-state-changed";

	public TruckController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
		super(id, commanders, config, eventBroker);

		int lowBatteryPercentageThreshold = config.getInt("truck.lowBatteryPercentageThreshold");
		int lowBatteryReportingInterval = config.getInt("truck.lowBatteryReportingInterval");

		batteryMonitor = new BatteryMonitor("The electric delivery truck", lowBatteryPercentageThreshold, lowBatteryReportingInterval);

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
		log.info("setting up truck controller");

		setupTruck();
		setupChargeVariance();
		setupIndicator();
		setupSolarPanel();
	}

	private void setupTruck() {
		String commanderName = config.getString("truck.commander");
		truckCommander = getCommanderByName(commanderName);
	}

	private void setupChargeVariance() {
		float maxVelocity = config.getFloat("truck.charge.variance.maxVelocity");
		float maxChangePercentage = config.getFloat("truck.charge.variance.maxChangePercentage");
		float minValue = config.getFloat("truck.charge.variance.minValue");
		float maxValue = config.getFloat("truck.charge.variance.maxValue");

		chargePowerVariance = new SimulatedVariance(
				maxVelocity,
				maxChangePercentage,
				minValue,
				maxValue
		);
	}

	private void setupIndicator() {
		String commanderName = config.getString("truck.indicator.commander");
		int port = config.getInt("truck.indicator.port");
		int channelCount = config.getInt("truck.indicator.channels");
		indicatorChannel = config.getInt("truck.indicator.channel");

		indicatorCommander = getCommanderByName(commanderName);
		indicatorDriver = new EtherioMultiDacActuator("Truck controller indicator led driver", indicatorCommander, port, channelCount);

		registerChild(indicatorDriver);
	}

	private void setupSolarPanel() {
		String commanderName = config.getString("truck.solar.commander");
		int port = config.getInt("truck.solar.port");
		int minInterval = config.getInt("truck.solar.minInterval");
		int pollInterval = config.getInt("truck.solar.pollInterval");
		float changeThreshold = config.getFloat("truck.solar.changeThreshold");

		truckBaseChargePower = config.getFloat("truck.charge.baseChargePower");

		log.debug("setting up solar panel on commander {} port {}", commanderName, port);

		Commander solarCommander = getCommanderByName(commanderName);

		solarPanelSensor = new EtherioAnalogInputSensor(
				"Truck solar panel",
				solarCommander,
				port,
				"kW",
				minInterval,
				pollInterval,
				changeThreshold
		);

		solarPanelSensor.addListener(this::onSolarPanelOutputChange);

		registerChild(solarPanelSensor);
	}

	@Override
	public void start() {
		super.start();

		log.info("starting truck controller");

		// listen for truck commander connectivity events
		truckCommander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to truck commander has been {}", isFirstConnect ? "established" : "re-established");

				if (isFirstConnect) {
					setupEventListeners();
				}

				requestBatteryVoltage();

				setIsRunning(true);
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.info("truck commander transport has been closed");

				setIsRunning(false);

				if (!isPlanned) {
					TextToSpeech.INSTANCE.speak("Wireless connection to the truck was lost, attempting to re-establish", false);
				}
			}
		});

		// listen for indicator commander connectivity events
		indicatorCommander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to indicator commander has been {}", isFirstConnect ? "established" : "re-established");

				// response to this will make the indicator output correct
				requestBatteryVoltage();
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.info("truck indicator transport has been closed");
			}
		});
	}

	@Override
	public void shutdown() {
		log.info("shutting down train controller");

		state.reset();
		updateState(state);

		super.shutdown();
	}

	private void requestBatteryVoltage() {
		truckCommander.sendCommand(COMMAND_GET_BATTERY_VOLTAGE).thenAccept((Commander.CommandResponse result) -> {
			boolean isCharging = result.response.getInt(0) == 1;
			float batteryVoltage = result.response.getFloat(1);
			int batteryChargePercentage = result.response.getInt(2);

			handleBatteryChargeStateChanged(isCharging, batteryVoltage, batteryChargePercentage);
		});
	}

	private void setupEventListeners() {
		log.debug("setting up event listeners");

		truckCommander.addRemoteCommandListener((Command command) -> {
			log.trace("got train event: {}", command.name);

			List<String> arguments = command.getArguments();

			for (int i = 0; i < arguments.size(); i++) {
				log.trace("- #{}: {}", i, arguments.get(i));
			}

			switch (command.name) {
				case EVENT_BATTERY_STATE_CHANGED: {
					boolean isCharging = command.getInt(0) == 1;
					float batteryVoltage = command.getFloat(1);
					int batteryChargePercentage = command.getInt(2);

					handleBatteryChargeStateChanged(isCharging, batteryVoltage, batteryChargePercentage);
					break;
				}

				default:
					log.warn("truck event '{}' is not handled", command.name);
					break;
			}
		});
	}

	private void handleBatteryChargeStateChanged(boolean isCharging, float batteryVoltage, int batteryChargePercentage) {
		log.debug("truck is now {}, battery voltage: {}V ({})", isCharging ? "charging" : "not charging", batteryVoltage, batteryChargePercentage);

		// check for charge state change
		if (state.getIsCharging() != isCharging) {
			if (isCharging) {
				handleTruckStartedCharging();
			} else {
				handleTruckStoppedCharging();
			}

			sendControlledActivatedEventIfNew();
		}

		state.setIsCharging(isCharging);
		state.setBatteryVoltage(batteryVoltage);
		state.setBatteryChargePercentage(batteryChargePercentage);

		indicatorDriver.setChannelValue(indicatorChannel, isCharging ? 0.0f : 1.0f);

		updateGridPower();

		batteryMonitor.checkForLowBattery(batteryChargePercentage, isCharging);
	}

	private void handleTruckStartedCharging() {
		TextToSpeech.INSTANCE.speak("Electric truck is now charging", true);

		// simulate periodic change of charging power
		clearChargeInterval();

		chargePowerInterval = Scheduler.setInterval(this::updateGridPower, CHARGE_POWER_INTERVAL_MS);
	}

	private void handleTruckStoppedCharging() {
		TextToSpeech.INSTANCE.speak("Electric truck is not charging any more, the battery is at " + state.getBatteryChargePercentage() + " percent", true);

		clearChargeInterval();
	}

	private void clearChargeInterval() {
		if (chargePowerInterval != null && !chargePowerInterval.isCancelled()) {
			chargePowerInterval.cancel(true);
		}
	}

	private void onSolarPanelOutputChange(float solarOutputPower) {
		log.trace("solar panel output changed to {}kW", solarOutputPower);

		updateGridPower();
	}

	private void updateGridPower() {
		boolean isCharging = state.getIsCharging();
		int batteryChargePercentage = state.getBatteryChargePercentage();
		float batteryVoltage = state.getBatteryVoltage();
		float solarOutputPower = solarPanelSensor.getState().getValue();
		float truckChargePower = calculateTruckChargePower(isCharging, batteryChargePercentage);
		float gridPowerBalance = calculateGridPowerBalance(truckChargePower, solarOutputPower);

		log.trace(
				"truck {}, solar panel output: {}kW, {} the grid: {}kW"
				, isCharging ? "charging at " + truckChargePower + "kW" : "not charging",
				solarOutputPower,
				gridPowerBalance > 0 ? "selling to" : "buying from",
				Math.abs(gridPowerBalance)
		);

		// update charge power state
		state.setSolarOutputPower(solarOutputPower);
		state.setChargePower(truckChargePower);
		state.setGridPowerBalance(gridPowerBalance);
		updateState(state);

		// report charge power and grid power balance measurements
		reportMeasurement(new ChargePowerMeasurement(truckChargePower, "kW"));
		reportMeasurement(new GridPowerBalanceMeasurement(gridPowerBalance, "kW"));

		// report battery state as well
		reportMeasurement(new BatteryMeasurement(batteryVoltage, batteryChargePercentage, isCharging));
	}

	private void setIsRunning(boolean isRunning) {
		state.setIsRunning(isRunning);

		updateState(state);
	}

	@SuppressWarnings("unused")
	private float calculateTruckChargePower(boolean isCharging, int batteryChargePercentage) {
		if (!isCharging) {
			return 0.0f;
		}

		// float actualChargePower = truckBaseChargePower * Util.map(batteryChargePercentage, 50.0f, 100.0f, 1.0f, 0.2f);
		float actualChargePower = truckBaseChargePower;
		float variance = chargePowerVariance.getUpdatedVariance();
		float chargePower = truckBaseChargePower + variance;

		log.trace("truck charge power: {}kW (actual: {}kW, variance: {}kW)", chargePower, actualChargePower, variance);

		return chargePower;
	}

	private float calculateGridPowerBalance(float truckChargePower, float solarPanelOutputPower) {
		return solarPanelOutputPower - truckChargePower;
	}

	private void sendControlledActivatedEventIfNew() {
		long timeSinceLastActivationReport = Util.since(lastActivationReportedTime);

		if (timeSinceLastActivationReport < CONTROLLER_ACTIVATION_MIN_PAUSE) {
			log.debug("new activation requested too soon ({}ms since last time), ignoring it", timeSinceLastActivationReport);

			return;
		}

		log.debug("reporting controller activation ({}ms since last time)", timeSinceLastActivationReport);

		reportEvent(new ControllerActivatedEvent());

		lastActivationReportedTime = Util.now();
	}
}
