package com.stagnationlab.c8y.driver.controllers;

import java.util.List;
import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioMultiDacActuator;
import com.stagnationlab.c8y.driver.fragments.controllers.Truck;
import com.stagnationlab.c8y.driver.measurements.BatteryMeasurement;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.c8y.driver.services.TextToSpeech;
import com.stagnationlab.etherio.Command;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;

@Slf4j
public class TruckController extends AbstractController {

	private final Truck state = new Truck();
	private Commander truckCommander;
	private Commander indicatorCommander;
	private AbstractMultiDacActuator indicatorDriver;
	private int indicatorChannel = 0;

	private static final String COMMAND_GET_BATTERY_VOLTAGE = "battery";
	private static final String EVENT_BATTERY_STATE_CHANGED = "battery-state-changed";

	public TruckController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
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
		log.info("setting up truck controller");

		setupTruck();
		setupIndicator();
	}

	private void setupTruck() {
		String commanderName = config.getString("truck.commander");
		truckCommander = getCommanderByName(commanderName);
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

		if (state.getIsCharging() != isCharging) {
			if (isCharging) {
				TextToSpeech.INSTANCE.speak("Electric truck is now charging", true);
			} else {
				TextToSpeech.INSTANCE.speak("Electric truck is not charging any more, the battery is at " + batteryChargePercentage + " percent", true);
			}
		}

		state.setIsCharging(isCharging);
		state.setBatteryVoltage(batteryVoltage);
		state.setBatteryChargePercentage(batteryChargePercentage);

		updateState(state);
		reportMeasurement(new BatteryMeasurement(batteryVoltage, batteryChargePercentage, isCharging));

		indicatorDriver.setChannelValue(indicatorChannel, isCharging ? 0.0f : 1.0f);
	}

	private void setIsRunning(boolean isRunning) {
		state.setIsRunning(isRunning);

		updateState(state);
	}
}
