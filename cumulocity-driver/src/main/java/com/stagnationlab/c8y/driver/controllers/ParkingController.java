package com.stagnationlab.c8y.driver.controllers;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.stagnationlab.c8y.driver.devices.AbstractMultiDacActuator;
import com.stagnationlab.c8y.driver.devices.AbstractTagSensor;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioMultiDacActuator;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioTagSensor;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.etherio.Commander;

import c8y.lx.driver.Driver;

public class ParkingController extends AbstractController {

	private static final Logger log = LoggerFactory.getLogger(ParkingController.class);

	private AbstractMultiDacActuator ledDriver;
	private Map<Integer, AbstractTagSensor> sensorsMap = new HashMap<>();
	private Map<Integer, Integer> ledChannelMap = new HashMap<>();
	private int slotCount = 0;

	public ParkingController(Map<String, Commander> commanders, Config config) {
		super(commanders, config);
	}

	@Override
	public List<Driver> initialize() {
		List<Driver> drivers = new ArrayList<>();

		setupSlotSensors(drivers);
		setupSlotIndicators(drivers);

		return drivers;
	}

	@Override
	public void start() {
		log.info("starting parking controller for {} slots", slotCount);

		for (int i = 1; i <= slotCount; i++) {
			setSlotFree(i, true);

			registerEventListeners(i, sensorsMap.get(i));
		}
	}

	private void setupSlotSensors(List<Driver> drivers) {
		slotCount = config.getInt("parking.slotCount");

		log.info("setting up parking controller for {} slots", slotCount);

		for (int i = 1; i <= slotCount; i++) {
			String commanderName = config.getString("parking.slot." + i + ".commander");
			int port = config.getInt("parking.slot." + i + ".port");
			int ledChannel = config.getInt("parking.slot." + i + ".ledChannel");

			Commander commander = commanders.get(commanderName);
			AbstractTagSensor sensor = new EtherioTagSensor("Parking slot sensor " + i, commander, port);

			sensorsMap.put(i, sensor);
			ledChannelMap.put(i, ledChannel);

			drivers.add(sensor);

			log.info("added parking slot sensor #{} on commander {} port {} using led channel {}", i, commanderName, port, ledChannel);
		}
	}

	private void setupSlotIndicators(List<Driver> drivers) {
		String commanderName = config.getString("parking.ledDriverCommander");
		int port = config.getInt("parking.ledDriverPort");
		int channelCount = config.getInt("parking.ledDriverChannels");

		log.info("setting up parking slot indicators on commander {} port {} with {} channels", commanderName, port, channelCount);

		Commander commander = commanders.get(commanderName);
		ledDriver = new EtherioMultiDacActuator("Parking led driver", commander, port, channelCount);

		drivers.add(ledDriver);
	}

	private void registerEventListeners(int index, AbstractTagSensor parkingSlotSensor) {
		parkingSlotSensor.addTagEventListener(new AbstractTagSensor.TagEventListener() {
			@Override
			public void onTagEnter(String tagName) {
				log.info("vehicle entered slot {}: {}", index, tagName);

				setSlotFree(index, false);
			}

			@Override
			public void onTagExit() {
				log.info("slot {} is now free", index);

				setSlotFree(index, true);
			}
		});
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
