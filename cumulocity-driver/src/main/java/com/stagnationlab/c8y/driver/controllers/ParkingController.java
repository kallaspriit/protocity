package com.stagnationlab.c8y.driver.controllers;

import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.AbstractTagSensor;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.etherio.EtherioTagSensor;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.TextToSpeech;
import com.stagnationlab.etherio.Commander;

public class ParkingController extends AbstractController {

	private static final Logger log = LoggerFactory.getLogger(ParkingController.class);

	private final com.stagnationlab.c8y.driver.fragments.ParkingController parkingController = new com.stagnationlab.c8y.driver.fragments.ParkingController();
	private AbstractMultiDacActuator ledDriver;
	private Map<Integer, AbstractTagSensor> sensorsMap = new HashMap<>();
	private Map<Integer, Integer> ledChannelMap = new HashMap<>();
	private int slotCount = 0;

	public ParkingController(String id, Map<String, Commander> commanders, Config config) {
		super(id, commanders, config);
	}

	@Override
	protected String getType() {
		return parkingController.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return parkingController;
	}

	@Override
	protected void setup() throws Exception {
		setupSlotSensors();
		setupSlotIndicators();
	}

	@Override
	public void start() {
		super.start();

		log.info("starting parking controller for {} slots", slotCount);

		for (int i = 1; i <= slotCount; i++) {
			setSlotFree(i, true);

			registerEventListeners(i, sensorsMap.get(i));
		}
	}

	private void setupSlotSensors() {
		slotCount = config.getInt("parking.slotCount");

		log.info("setting up parkingController controller for {} slots", slotCount);

		for (int i = 1; i <= slotCount; i++) {
			String commanderName = config.getString("parking.slot." + i + ".commander");
			int port = config.getInt("parking.slot." + i + ".port");
			int ledChannel = config.getInt("parking.slot." + i + ".ledChannel");

			Commander commander = commanders.get(commanderName);
			AbstractTagSensor sensor = new EtherioTagSensor("ParkingController slot sensor " + i, commander, port);

			sensorsMap.put(i, sensor);
			ledChannelMap.put(i, ledChannel);

			registerChild(sensor);

			log.info("added parkingController slot sensor #{} on commander {} port {} using led channel {}", i, commanderName, port, ledChannel);
		}
	}

	private void setupSlotIndicators() {
		String commanderName = config.getString("parking.ledDriverCommander");
		int port = config.getInt("parking.ledDriverPort");
		int channelCount = config.getInt("parking.ledDriverChannels");

		log.info("setting up parkingController slot indicators on commander {} port {} with {} channels", commanderName, port, channelCount);

		Commander commander = commanders.get(commanderName);
		ledDriver = new EtherioMultiDacActuator("ParkingController led driver", commander, port, channelCount);

		registerChild(ledDriver);
	}

	private void registerEventListeners(int index, AbstractTagSensor parkingSlotSensor) {
		parkingSlotSensor.addTagEventListener(new AbstractTagSensor.TagEventListener() {
			@Override
			public void onTagEnter(String tagName) {
				log.info("vehicle entered slot {}: {}", index, tagName);

				setSlotFree(index, false);
				playSlotTakenSound(index, tagName);
			}

			@Override
			public void onTagExit() {
				log.info("slot {} is now free", index);

				setSlotFree(index, true);
				playSlotFreedSound(index);
			}
		});
	}

	private void playSlotTakenSound(int index, String vehicleName) {
		String message = vehicleName.substring(0, 1).toUpperCase() + vehicleName.substring(1).toLowerCase() + " is now parked on slot " + index;

		TextToSpeech.speak(message);
	}

	private void playSlotFreedSound(int index) {
		String message = "Slot " + index + " is now free";

		TextToSpeech.speak(message);
	}

	private void setSlotFree(int index, boolean isFree) {
		int ledChannel = ledChannelMap.get(index);

		if (isFree) {
			ledDriver.setChannelValue(ledChannel, 1.0f);
		} else {
			ledDriver.setChannelValue(ledChannel, 0.0f);
		}

		log.debug("setting slot {} indicator to show {}", index, isFree ? "free" : "occupied");
	}


}
