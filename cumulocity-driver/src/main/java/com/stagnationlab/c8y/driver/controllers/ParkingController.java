package com.stagnationlab.c8y.driver.controllers;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.stagnationlab.c8y.driver.devices.AbstractTagSensor;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioTagSensor;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.etherio.Commander;

import c8y.lx.driver.Driver;

public class ParkingController extends AbstractController {

	private static final Logger log = LoggerFactory.getLogger(ParkingController.class);

	private final Map<Integer, AbstractTagSensor> parkingSlotSensors = new HashMap<>();

	public ParkingController(Map<String, Commander> commanders, Config config) {
		super(commanders, config);
	}

	@Override
	public List<Driver> setup() {
		List<Driver> drivers = new ArrayList<>();
		int slotCount = config.getInt("parking.slotCount");

		log.info("setting up parking devices for {} slots", slotCount);

		for (int i = 1; i <= slotCount; i++) {
			String commanderName = config.getString("parking.slot." + i + ".commanderName");
			int port = config.getInt("parking.slot." + i + ".port");

			Commander commander = commanders.get(commanderName);
			AbstractTagSensor parkingSlotSensor = new EtherioTagSensor("Parking slot sensor " + i, commander, port);

			registerEventListeners(i, parkingSlotSensor);

			parkingSlotSensors.put(i, parkingSlotSensor);

			drivers.add(parkingSlotSensor);

			log.info("added parking slot sensor #{} on commander {} port {}", i, commanderName, port);
		}

		return drivers;
	}

	private void registerEventListeners(int index, AbstractTagSensor parkingSlotSensor) {
		parkingSlotSensor.addTagEventListener(new AbstractTagSensor.TagEventListener() {
			@Override
			public void onTagEnter(String tagName) {
				log.info("tag for slot {} entered: {}", index, tagName);
			}

			@Override
			public void onTagExit() {
				log.info("tag for slot {} exited", index);
			}
		});
	}
}
