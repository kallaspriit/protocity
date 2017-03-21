package com.stagnationlab.c8y.driver.services;

import static java.util.concurrent.TimeUnit.MILLISECONDS;

import java.util.Date;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;

import lombok.extern.slf4j.Slf4j;

@Slf4j
public class Scheduler {

	private static int intervalCount = 0;

	public static ScheduledFuture<?> setInterval(Runnable runnable, long intervalMs) {
		log.info("creating an interval every {}ms", intervalMs);

		ScheduledExecutorService executorService = Executors.newSingleThreadScheduledExecutor(
				r -> new Thread(r,  "Interval" + (intervalCount++))
		);

		long now = new Date().getTime();
		long initialDelay = intervalMs - (now % intervalMs);

		return executorService.scheduleAtFixedRate(
				runnable,
				initialDelay,
				intervalMs,
				MILLISECONDS
		);
	}

}
