package com.stagnationlab.c8y.driver;

import c8y.lx.driver.Driver;
import c8y.lx.driver.OperationExecutor;
import com.cumulocity.model.idtype.GId;
import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.cumulocity.sdk.client.Platform;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioAnalogInputSensor;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioButtonSensor;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioLightSensor;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioMonitoringSensor;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioMotionSensor;
import com.stagnationlab.c8y.driver.platforms.etherio.EtherioRelayActuator;
import com.stagnationlab.c8y.driver.platforms.simulated.SimulatedLightSensor;
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
public class Gateway implements Driver, OperationExecutor {
    private static final Logger log = LoggerFactory.getLogger(Gateway.class);

    private final List<Driver> drivers = new ArrayList<>();
    private GId gid;

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

        for (Driver driver : drivers) {
            try {
                driver.initializeInventory(managedObjectRepresentation);
            } catch (Throwable e) {
                log.warn("initializing driver {} inventory failed with {} ({})", driver.getClass().getSimpleName(), e.getClass().getSimpleName(), e.getMessage());
            }
        }
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
        return "c8y_Restart";
    }

    @Override
    public void execute(OperationRepresentation operation, boolean cleanup) throws Exception {
        log.info("execution requested{}", cleanup ? " (cleaning up)" : "");

        if (!this.gid.equals(operation.getDeviceId())) {
            return;
        }

        if (cleanup) {
            operation.setStatus(OperationStatus.FAILED.toString());

            return;
        }


        log.info("restarting...");

        operation.setStatus(OperationStatus.SUCCESSFUL.toString());

        new ProcessBuilder(new String[]{"shutdown", "-r"}).start().waitFor();
    }

    @Override
    public OperationExecutor[] getSupportedOperations() {
        List<OperationExecutor> operationExecutorsList = new ArrayList<>();

        operationExecutorsList.add(this);

        for (Driver driver : drivers) {
            for (OperationExecutor driverOperationExecutor : driver.getSupportedOperations()) {
                operationExecutorsList.add(driverOperationExecutor);
            }
        }

        return operationExecutorsList.toArray(new OperationExecutor[operationExecutorsList.size()]);
    }

    private void registerDriver(Driver driver) {
        log.info("registering driver '{}'", driver.getClass().getSimpleName());

        drivers.add(driver);
    }

    private void initializeDrivers() {
        log.info("initializing drivers ({} total)", drivers.size());

        Iterator<Driver> iterator = drivers.iterator();

        while (iterator.hasNext()) {
            Driver driver = iterator.next();

            try {
                log.info("initializing driver '{}'", driver.getClass().getSimpleName());

                driver.initialize();
            } catch (Throwable e) {
                log.warn("initializing driver '{}' failed with '{}' ({}), skipping it", driver.getClass().getSimpleName(), e.getClass().getSimpleName(), e.getMessage());

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
                log.warn("initializing driver '{}' platform failed with '{}' ({}), skipping it", driver.getClass().getSimpleName(), e.getClass().getSimpleName(), e.getMessage());

                iterator.remove();
            }
        }
    }

    private void setupEtherio() throws IOException {
        // TODO make host and port configurable, support multiple
        String hostName = "10.220.20.17";
        int portNumber = 8080;

        SocketClient socketClient = new SocketClient(hostName, portNumber);
        socketClient.connect();

        commander = new Commander(socketClient);
    }

    private void setupDevices() {
        log.info("setting up sensors");

        // EtherIO devices
        // setupEtherioLightSensor();
        setupEtherioRelayActuator();
        setupEtherioButtonSensor();
        setupEtherioMonitoringSensor();
        setupEtherioAnalogInputSensor();
        setupEtherioMotionSensor();

        // simulated devices
        // setupSimulatedLightSensor();
        // setupSimulatedMotionSensor();
        // setupSimulatedRelayActuator();
    }

    private void setupSimulatedLightSensor() {
        registerDriver(
                new SimulatedLightSensor("Simulated light sensor")
        );
    }

    private void setupSimulatedRelayActuator() {
        registerDriver(
                new SimulatedRelayActuator("Simulated relay")
        );
    }

    private void setupEtherioLightSensor() {
        // TODO make port configurable
        registerDriver(
                new EtherioLightSensor("EtherIO light sensor", commander, 6)
        );
    }

    private void setupEtherioRelayActuator() {
        // TODO make port configurable
        registerDriver(
                new EtherioRelayActuator("EtherIO relay", commander, 1)
        );
    }

    private void setupEtherioButtonSensor() {
        // TODO make port configurable
        registerDriver(
                new EtherioButtonSensor("EtherIO button", commander, 4)
        );
    }

    private void setupEtherioMonitoringSensor() {
        registerDriver(
                new EtherioMonitoringSensor("EtherIO monitor", commander)
        );
    }

    private void setupEtherioAnalogInputSensor() {
        registerDriver(
                new EtherioAnalogInputSensor("EtherIO analog input", commander, 6, "%")
        );
    }

    private void setupEtherioMotionSensor() {
        registerDriver(
                new EtherioMotionSensor("EtherIO motion sensor", commander, 5)
        );
    }
}
