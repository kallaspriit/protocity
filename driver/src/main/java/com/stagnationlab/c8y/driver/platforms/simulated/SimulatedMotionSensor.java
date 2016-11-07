package com.stagnationlab.c8y.driver.platforms.simulated;

import c8y.Hardware;
import com.stagnationlab.c8y.driver.devices.AbstractMotionSensor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Date;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.ThreadFactory;

import static java.util.concurrent.Executors.newSingleThreadScheduledExecutor;
import static java.util.concurrent.TimeUnit.MILLISECONDS;

public class SimulatedMotionSensor extends AbstractMotionSensor implements Runnable {

    private static final Logger log = LoggerFactory.getLogger(SimulatedMotionSensor.class);

    @SuppressWarnings("FieldCanBeLocal")
    private final long MEASUREMENT_INTERVAL = 30000;

    private final ScheduledExecutorService executorService;
    private volatile ScheduledFuture<?> scheduledFuture;

    public SimulatedMotionSensor(String id) {
        super(id);

        this.executorService = newSingleThreadScheduledExecutor(
                new NamedThreadFactory("SimulatedMotionSensor")
        );
    }

    @Override
    public void start() {
        scheduleMeasurements();
    }

    @Override
    protected Hardware getHardware() {
        return new Hardware(
                "Simulated Motion Sensor",
                "006226662342449",
                "1.0.0"
        );
    }

    @Override
    public void run() {
        triggerMotionDetected();

        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        triggerMotionEnded();
    }

    private static long computeInitialDelay(long now, long pollingInterval) {
        long remainder = now % pollingInterval;
        return pollingInterval - remainder;
    }

    private void scheduleMeasurements() {
        log.info("scheduling measurements");

        if (scheduledFuture != null) {
            return; // already started
        }

        long now = new Date().getTime();
        long initialDelay = computeInitialDelay(now, MEASUREMENT_INTERVAL);

        scheduledFuture = executorService.scheduleAtFixedRate(this, initialDelay, MEASUREMENT_INTERVAL, MILLISECONDS);
    }

    private static class NamedThreadFactory implements ThreadFactory {

        private final String threadName;

        NamedThreadFactory(String threadName) {
            this.threadName = threadName;
        }

        @Override
        public Thread newThread(Runnable r) {
            return new Thread(r, threadName);
        }
    }
}
