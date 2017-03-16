package com.stagnationlab.c8y.driver.controllers;

import java.util.HashMap;
import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.AbstractTagSensor;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioTagSensor;
import com.stagnationlab.c8y.driver.fragments.controllers.Parking;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.c8y.driver.services.TextToSpeech;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;

@Slf4j
public class ParkingController extends AbstractController {

	private final Parking state = new Parking();
	private AbstractMultiDacActuator ledDriver;
	private final Map<Integer, AbstractTagSensor> sensorsMap = new HashMap<>();
	private final Map<Integer, Integer> ledChannelMap = new HashMap<>();
	private final Map<Integer, String> slotNameMap = new HashMap<>();
	private Commander slotIndicatorsCommander = null;
	private int slotCount = 0;

	public ParkingController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
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
		log.info("setting up parking controller");

		setupSlotSensors();
		setupSlotIndicators();
	}

	@Override
	public void start() {
		super.start();

		log.info("starting parking controller");

		slotIndicatorsCommander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onOpen(boolean isFirstConnect) {
				log.debug("connection to parking slot indicators commander has been {}", isFirstConnect ? "established" : "re-established");

				for (int i = 0; i < slotCount; i++) {
					if (isFirstConnect) {
						registerEventListeners(i, sensorsMap.get(i));
					}

					// indicate initially free slot
					setSlotFree(i, true);
				}
			}
		});

		// update initial state
		state.setIsRunning(true);
		updateState(state);
	}

	@Override
	public void shutdown() {
		log.info("shutting down parking controller");

		state.reset();
		updateState(state);

		super.shutdown();
	}

	private void setupSlotSensors() {
		slotCount = config.getInt("parking.slotCount");

		log.debug("setting up {} slot tag sensors", slotCount);

		for (int i = 0; i < slotCount; i++) {
			String name = config.getString("parking.slot." + i + ".name");
			String commanderName = config.getString("parking.slot." + i + ".commander");
			int port = config.getInt("parking.slot." + i + ".port");
			int ledChannel = config.getInt("parking.slot." + i + ".ledChannel");

			Commander commander = getCommanderByName(commanderName);
			AbstractTagSensor sensor = new EtherioTagSensor("Parking controller slot sensor " + i, commander, port);

			sensorsMap.put(i, sensor);
			ledChannelMap.put(i, ledChannel);
			slotNameMap.put(i, name);

			registerChild(sensor);

			Parking.SlotState slotState = new Parking.SlotState(i);
			state.addSlot(slotState);

			log.debug("added parking controller slot sensor #{} called '{}' on commander {} port {} using led channel {}", i, name, commanderName, port, ledChannel);
		}
	}

	private void setupSlotIndicators() {
		String commanderName = config.getString("parking.ledDriverCommander");
		int port = config.getInt("parking.ledDriverPort");
		int channelCount = config.getInt("parking.ledDriverChannels");

		log.debug("setting up slot indicators on commander {} port {} with {} channels", commanderName, port, channelCount);

		slotIndicatorsCommander = getCommanderByName(commanderName);
		ledDriver = new EtherioMultiDacActuator("Parking controller led driver for commander " + commanderName, slotIndicatorsCommander, port, channelCount);

		registerChild(ledDriver);
	}

	private void registerEventListeners(int index, AbstractTagSensor parkingSlotSensor) {
		parkingSlotSensor.addTagEventListener(new AbstractTagSensor.TagEventListener() {
			@Override
			public void onTagEnter(String tagName) {
				occupy(index, tagName);
			}

			@Override
			public void onTagExit() {
				free(index);
			}

			@Override
			public void onTagUidEvent(String action, String uid) {
				// ignore
			}
		});
	}

	private String getSlotName(int index) {
		return slotNameMap.get(index);
	}

	private void occupy(int index, String occupantName) {
		Parking.SlotState slotState = state.slotByIndex(index);

		if (slotState == null) {
			log.warn("marking slot #{} as occupied by {} but no such slot was found, ignoring it", index, occupantName);

			return;
		}

		log.debug("slot #{} is now occupied by {}", index, occupantName);

		slotState.occupy(occupantName);

		updateState(state);

		setSlotFree(index, false);
		playSlotTakenSound(index, occupantName);
	}

	private void free(int index) {
		Parking.SlotState slotState = state.slotByIndex(index);

		if (slotState == null) {
			log.warn("marking slot #{} as free but no such slot was found, ignoring it", index);

			return;
		}

		log.debug("slot #{} is now free", index);

		slotState.free();

		updateState(state);

		setSlotFree(index, true);
		playSlotFreedSound(index);
	}

	private void setSlotFree(int index, boolean isFree) {
		int ledChannel = ledChannelMap.get(index);

		if (isFree) {
			ledDriver.setChannelValue(ledChannel, 1.0f);
		} else {
			ledDriver.setChannelValue(ledChannel, 0.0f);
		}

		log.trace("setting slot {} indicator to show {}", index, isFree ? "free" : "occupied");
	}

	private void playSlotTakenSound(int index, String vehicleName) {
		String name = getSlotName(index);
		String message = vehicleName.substring(0, 1).toUpperCase() + vehicleName.substring(1).toLowerCase() + " is now parked on " + name;

		TextToSpeech.INSTANCE.speak(message, true);
	}

	private void playSlotFreedSound(int index) {
		String name = getSlotName(index);
		String message = name + " is now free";

		TextToSpeech.INSTANCE.speak(message, true);
	}

}
