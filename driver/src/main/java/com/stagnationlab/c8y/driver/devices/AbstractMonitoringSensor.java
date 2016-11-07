package com.stagnationlab.c8y.driver.devices;


import c8y.Hardware;
import c8y.lx.driver.MeasurementPollingDriver;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.stagnationlab.c8y.driver.services.DeviceManager;
import com.stagnationlab.c8y.driver.measurements.DeviceMonitoringMeasurement;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public abstract class AbstractMonitoringSensor extends MeasurementPollingDriver {

    class MonitoringSensor {}

    static class MonitoringStatus {
        double totalMemory;
        double usedMemory;
        double freeMemory;
        double sharedMemory;
        double bufferedMemory;
        double cachedMemory;

        MonitoringStatus(
                double totalMemory,
                double usedMemory,
                double freeMemory,
                double sharedMemory,
                double bufferedMemory,
                double cachedMemory
        ) {
            this.totalMemory = totalMemory;
            this.usedMemory = usedMemory;
            this.freeMemory = freeMemory;
            this.sharedMemory = sharedMemory;
            this.bufferedMemory = bufferedMemory;
            this.cachedMemory = cachedMemory;
        }
    }

    private static final Logger log = LoggerFactory.getLogger(AbstractMonitoringSensor.class);

    private static final String TYPE = "Monitoring";

    private final String id;

    protected AbstractMonitoringSensor(String id) {
        super(TYPE + "Sensor", TYPE.toLowerCase(), 5000);

        this.id = id;
    }

    @Override
    public void initialize() throws Exception {
        log.info("initializing");
    }

    @Override
    public void discoverChildren(ManagedObjectRepresentation parent) {
        log.info("creating child");

        ManagedObjectRepresentation childDevice = DeviceManager.createChild(
                id,
                TYPE,
                getPlatform(),
                parent,
                getHardware(),
                getSupportedOperations(),
                new MonitoringSensor()
        );

        setSource(childDevice);
    }

    @Override
    public void run() {
        MonitoringStatus monitoringStatus = getMonitoringStatus();

        if (monitoringStatus == null) {
            log.warn("getting monitoring status failed");

            return;
        }

        DeviceMonitoringMeasurement deviceMonitoringMeasurement = new DeviceMonitoringMeasurement();
        deviceMonitoringMeasurement.update(
                monitoringStatus.totalMemory,
                monitoringStatus.usedMemory,
                monitoringStatus.freeMemory,
                monitoringStatus.sharedMemory,
                monitoringStatus.bufferedMemory,
                monitoringStatus.cachedMemory
        );

        sendMeasurement(deviceMonitoringMeasurement);
    }

    protected abstract Hardware getHardware();

    protected abstract MonitoringStatus getMonitoringStatus();
}
