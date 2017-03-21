package com.stagnationlab.c8y.driver.services;

import java.util.concurrent.ScheduledFuture;

import lombok.extern.slf4j.Slf4j;

@Slf4j
public class BatteryMonitor {

	private final String entityName;
	private final int lowBatteryPercentageThreshold;
	private final int lowBatteryReportingInterval;
	private ScheduledFuture<?> lowBatteryInterval;
	private int lastBatteryPercentage = 0;

	public BatteryMonitor(String entityName, int lowBatteryPercentageThreshold, int lowBatteryReportingInterval) {
		this.entityName = entityName;
		this.lowBatteryPercentageThreshold = lowBatteryPercentageThreshold;
		this.lowBatteryReportingInterval = lowBatteryReportingInterval;
	}

	public void checkForLowBattery(int batteryChargePercentage) {
		lastBatteryPercentage = batteryChargePercentage;

		if (batteryChargePercentage > lowBatteryPercentageThreshold) {
			clearLowBatteryInterval();
		} else {
			startLowBatteryInterval();
		}
	}

	private void startLowBatteryInterval() {
		clearLowBatteryInterval();

		if (lowBatteryInterval == null || lowBatteryInterval.isCancelled() || lowBatteryInterval.isDone()) {
			log.debug("starting low battery interval");

			lowBatteryInterval = Scheduler.setInterval(this::reportLowBattery, lowBatteryReportingInterval);
		}
	}

	private void clearLowBatteryInterval() {
		if (lowBatteryInterval != null && !lowBatteryInterval.isCancelled()) {
			log.debug("clearing low battery reporting interval");

			lowBatteryInterval.cancel(true);
		}
	}

	private void reportLowBattery() {
		log.info("{} has low battery ({}%)", entityName, lastBatteryPercentage);

		TextToSpeech.INSTANCE.speak(entityName + " has low battery of " + lastBatteryPercentage + "%, please charge it", false);
	}
}
