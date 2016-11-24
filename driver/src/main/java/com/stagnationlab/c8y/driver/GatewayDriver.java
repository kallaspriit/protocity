package com.stagnationlab.c8y.driver;

import c8y.lx.driver.Driver;
import c8y.lx.driver.OperationExecutor;
import com.cumulocity.model.idtype.GId;
import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.cumulocity.sdk.client.Platform;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioRelayActuator;
import com.stagnationlab.c8y.driver.platforms.simulated.SimulatedLightSensor;
import com.stagnationlab.c8y.driver.platforms.simulated.SimulatedMotionSensor;
import com.stagnationlab.c8y.driver.platforms.simulated.SimulatedRelayActuator;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.SocketClient;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;


@SuppressWarnings("unused")
public class GatewayDriver implements Driver, OperationExecutor {
    private static final Logger log = LoggerFactory.getLogger(GatewayDriver.class);

    private final List<Driver> drivers = new ArrayList<>();
    private GId gid;

    // EtherIO
    private SocketClient socketClient;
    private Commander commander;

    @Override
    public void initialize() throws Exception {
        log.info("initializing");

        setupEtherio();
        setupDevices();

        try {
            initializeDrivers();
        } catch (Exception e) {
            log.warn("initializing drivers failed");
        }
    }

    @Override
    public void initialize(Platform platform) throws Exception {
        log.info("initializing platform");

        try {
            initializeDrivers(platform);
        } catch (Exception e) {
            log.warn("initializing drivers platform failed");
        }
    }

    @Override
    public void initializeInventory(ManagedObjectRepresentation managedObjectRepresentation) {
        log.info("initializing inventory");
    }

    @Override
    public void discoverChildren(ManagedObjectRepresentation managedObjectRepresentation) {
        log.info("discovering children");

        this.gid = managedObjectRepresentation.getId();

        for (Driver driver : drivers) {
            driver.discoverChildren(managedObjectRepresentation);
        }
    }

    @Override
    public void start() {
        log.info("starting driver");

        for (Driver driver : drivers) {
            driver.start();
        }
    }

    @Override
    public String supportedOperationType() {
        log.info("supported operation type requested");

        return "c8y_Restart";
    }

    @Override
    public void execute(OperationRepresentation operation, boolean cleanup) throws Exception {
        log.info("execution requested (cleanup: " + (cleanup ? "yes" : "no") + ")");

        if (!this.gid.equals(operation.getDeviceId())) {
            // Silently ignore the operation if it is not targeted to us, another driver will (hopefully) care.
            return;
        }

        if (cleanup) {
            operation.setStatus(OperationStatus.SUCCESSFUL.toString());
        } else {
            log.info("shutting down");

            new ProcessBuilder(new String[]{"shutdown", "-r"}).start().waitFor();
        }
    }

    @Override
    public OperationExecutor[] getSupportedOperations() {
        log.info("supported operations requested");

        List<OperationExecutor> operationExecutorsList = new ArrayList<>();

        operationExecutorsList.add(this);

        for (Driver driver : drivers) {
            for (OperationExecutor driverOperationExecutor : driver.getSupportedOperations()) {
                operationExecutorsList.add(driverOperationExecutor);
            }
        }

        return operationExecutorsList.toArray(new OperationExecutor[operationExecutorsList.size()]);
    }

    private void initializeDrivers() {
        log.info("initializing drivers");

        Iterator<Driver> iterator = drivers.iterator();

        while (iterator.hasNext()) {
            Driver driver = iterator.next();

            try {
                log.info("initializing driver " + driver.getClass().getName());

                driver.initialize();
            } catch (Throwable e) {
                log.warn("initializing driver failed with " + e.getClass().getName() + " (" + e.getMessage() + "), skipping the driver " + driver.getClass().getName());

                iterator.remove();
            }
        }
    }

    private void initializeDrivers(Platform platform) {
        log.info("initializing drivers with platform");

        Iterator<Driver> iterator = drivers.iterator();

        while (iterator.hasNext()) {
            Driver driver = iterator.next();

            try {
                driver.initialize(platform);
            } catch (Throwable e) {
                log.warn("initializing driver platform failed with " + e.getClass().getName() + " (" + e.getMessage() + "), skipping the driver " + driver.getClass().getName());

                iterator.remove();
            }
        }
    }

    private void setupEtherio() throws IOException {
        // TODO make configurable
        String hostName = "10.220.20.17";
        int portNumber = 8080;

        socketClient = new SocketClient(hostName, portNumber);
        socketClient.connect();

        commander = new Commander(socketClient);
    }

    private void setupDevices() {
        log.info("setting up sensors");

        setupEtherioRelayActuator();

        setupSimulatedLightSensor();
        setupSimulatedMotionSensor();

        setupSimulatedRelayActuator();
        setupEtherioRelayActuator();
    }

    private void setupSimulatedLightSensor() {
        log.info("setting up simulated light sensor");

        drivers.add(
                new SimulatedLightSensor("1")
        );
    }

    private void setupSimulatedMotionSensor() {
        log.info("setting up simulated motion sensor");

        drivers.add(
                new SimulatedMotionSensor("1")
        );
    }

    private void setupSimulatedRelayActuator() {
        log.info("setting up simulated relay actuator");

        drivers.add(
                new SimulatedRelayActuator("1")
        );
    }

    private void setupEtherioRelayActuator() {
        log.info("setting up EtherIO relay actuator");

        drivers.add(
                new EtherioRelayActuator("1", commander, 1)
        );
    }
}
