package com.stagnationlab.c8y.driver;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.cumulocity.model.idtype.GId;
import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.cumulocity.sdk.client.Platform;
import com.stagnationlab.c8y.driver.controllers.AbstractController;
import com.stagnationlab.c8y.driver.controllers.ParkingController;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.TextToSpeech;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.SocketClient;

import c8y.lx.driver.Driver;
import c8y.lx.driver.OperationExecutor;

@SuppressWarnings({ "unused", "SameParameterValue" })
public class Gateway implements Driver, OperationExecutor {
	private static final Logger log = LoggerFactory.getLogger(Gateway.class);

	private final Config config = new Config();
	private final List<Driver> drivers = new ArrayList<>();
	private final List<AbstractController> controllers = new ArrayList<>();
	private final Map<String, Commander> commanders = new HashMap<>();
	private GId globalId;

	private static final int DEFAULT_CONTROLLER_PORT = 8080;
	private static final String CONFIG_FILENAME = "config.properties";

	@Override
	public void initialize() throws Exception {
		log.info("setting up");

		TextToSpeech.speak("Setting everything up, please wait");

		try {
			setupConfig();
			setupCommanders();
			setupControllers();
		} catch (Exception e) {
			log.warn("initialize failed! ({} - {})", e.getClass().getSimpleName(), e.getMessage());

			e.printStackTrace();

			throw e;
		}

		try {
			initializeDrivers();
		} catch (Exception e) {
			log.warn("initializing drivers failed! ({} - {})", e.getClass().getSimpleName(), e.getMessage());

			e.printStackTrace();

			throw e;
		}
	}

	private void setupConfig() throws IOException {
		log.info("setting up config from src/main/resources/{}", CONFIG_FILENAME);

		InputStream configInputStream = getClass().getClassLoader().getResourceAsStream(CONFIG_FILENAME);

		config.load(configInputStream);
	}

	private void setupCommanders() throws IOException {
		log.info("setting up commanders");

		commanders.put("1", createCommander("1"));
		commanders.put("2", createCommander("2"));
		commanders.put("3", createCommander("3"));
		commanders.put("4", createCommander("4"));
		//commanders.put("train", createCommander("train"));
	}

	private void setupControllers() {
		log.info("setting up controllers");

		registerController(
				new ParkingController(commanders, config)
		);

		for (AbstractController controller : controllers) {
			List<Driver> drivers = controller.initialize();

			for (Driver driver : drivers) {
				registerDriver(driver);
			}
		}
	}

	@Override
	public void initialize(Platform platform) throws Exception {
		log.info("initializing");

		initializeDrivers(platform);
		initializeControllers(platform);
	}

	private void initializeDrivers() {
		log.info("setting up drivers ({} total)", drivers.size());

		Iterator<Driver> iterator = drivers.iterator();

		while (iterator.hasNext()) {
			Driver driver = iterator.next();

			try {
				log.info("setting up driver '{}'", driver.getClass().getSimpleName());

				driver.initialize();
			} catch (Throwable e) {
				log.warn("setting up driver '{}' failed with '{}' ({}), skipping it", driver.getClass().getSimpleName(), e.getClass().getSimpleName(), e.getMessage());

				iterator.remove();
			}
		}
	}

	private void initializeDrivers(Platform platform) {
		log.info("initializing drivers ({} total)", drivers.size());

		Iterator<Driver> iterator = drivers.iterator();

		while (iterator.hasNext()) {
			Driver driver = iterator.next();

			try {
				log.info("initializing driver '{}'", driver.getClass().getSimpleName());

				driver.initialize(platform);
			} catch (Throwable e) {
				log.warn("initializing driver '{}' platform failed with '{}' ({}), skipping it", driver.getClass().getSimpleName(), e.getClass().getSimpleName(), e.getMessage());

				iterator.remove();
			}
		}
	}

	private void initializeControllers(Platform platform) {
		log.info("initializing controllers ({} total)", controllers.size());

		for (AbstractController controller : controllers) {
			controller.initialize(platform);
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
		log.debug("discovering children");

		this.globalId = managedObjectRepresentation.getId();

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

		for (AbstractController controller : controllers) {
			controller.start();
		}

		TextToSpeech.speak("Ready, welcome to Telia Lego City!");
	}

	@Override
	public String supportedOperationType() {
		return "c8y_Restart";
	}

	@Override
	public void execute(OperationRepresentation operation, boolean cleanup) throws Exception {
		log.debug("execution requested{}", cleanup ? " (cleaning up)" : "");

		if (!this.globalId.equals(operation.getDeviceId())) {
			return;
		}

		if (cleanup) {
			operation.setStatus(OperationStatus.FAILED.toString());

			return;
		}

		log.info("restarting...");

		operation.setStatus(OperationStatus.SUCCESSFUL.toString());

		new ProcessBuilder(new String[] { "shutdown", "-r" }).start().waitFor();
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

	private void registerController(AbstractController controller) {
		log.info("registering controller '{}'", controller.getClass().getSimpleName());

		controllers.add(controller);
	}

	/*
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

	private void setupEtherioRelayActuator() {
		// TODO make port configurable
		registerDriver(
				new EtherioRelayActuator("EtherIO relay", controllerCommander, 1)
		);
	}

	private void setupEtherioButtonSensor() {
		// TODO make port configurable
		registerDriver(
				new EtherioButtonSensor("EtherIO button", controllerCommander, 4)
		);
	}

	private void setupEtherioMonitoringSensor() {
		registerDriver(
				new EtherioMonitoringSensor("EtherIO monitor", controllerCommander)
		);
	}

	private void setupEtherioAnalogInputSensor() {
		// TODO make port configurable
		registerDriver(
				new EtherioAnalogInputSensor("EtherIO analog input", controllerCommander, 6, "%")
		);
	}

	private void setupEtherioMotionSensor() {
		// TODO make port configurable
		registerDriver(
				new EtherioMotionSensor("EtherIO motion sensor", controllerCommander, 5)
		);
	}

	private void setupEtherioLightSensor() {
		// TODO make port configurable
		registerDriver(
				new EtherioLightSensor("EtherIO light sensor", controllerCommander, 6)
		);
	}

	private void setupEtherioTemperatureSensor() {
		// TODO make port configurable
		registerDriver(
				new EtherioTemperatureSensor("EtherIO temperature sensor", controllerCommander, 6)
		);
	}

	private void setupEtherioTagSensor() {
		// TODO make port configurable
		registerDriver(
				new EtherioTagSensor("EtherIO tag sensor", controllerCommander, 2)
		);
	}

	private void setupEtherioMultiDacActuator() {
		// TODO make port configurable
		registerDriver(
				new EtherioMultiDacActuator("EtherIO multiple DAC actuator", controllerCommander, 3, 16)
		);
	}
	*/

	private Commander createCommander(String id) {
		String host = config.getString("commander." + id + ".host");
		int port = config.getInt("commander.\" + id + \".port", DEFAULT_CONTROLLER_PORT);

		log.info("connecting to controller commander {} at {}:{}", id, host, port);

		SocketClient socketClient = new SocketClient(host, port);

		socketClient.addMessageListener(new MessageTransport.MessageListener() {
			@Override
			public void onSocketOpen() {
				log.info("socket of controller {} at {}:{} was opened", id, host, port);
			}

			@Override
			public void onSocketClose() {
				log.info("socket of controller {} at {}:{} was closed", id, host, port);
			}

			@Override
			public void onSocketMessageReceived(String message) {
				// don't log heartbeat messages
				if (message.length() >= 11 && message.substring(0, 11).equals("0:HEARTBEAT")) {
					return;
				}

				log.debug("got controller {} message: '{}'", id, message);
			}
		});

		Commander commander = new Commander(socketClient);

		try {
			socketClient.connect();
		} catch (IOException e) {
			log.warn("connecting to controller {} at {}:{} failed ({} - {})", id, host, port, e.getClass().getSimpleName(), e.getMessage());
		}

		commander.sendCommand("version").thenAccept(commandResponse -> log.info("got commander {} version: {}", id, commandResponse.response.getString(0)));

		return commander;
	}
}
