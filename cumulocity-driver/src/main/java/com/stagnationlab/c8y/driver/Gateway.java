package com.stagnationlab.c8y.driver;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.stagnationlab.c8y.driver.controllers.AbstractController;
import com.stagnationlab.c8y.driver.controllers.LightingController;
import com.stagnationlab.c8y.driver.controllers.ParkingController;
import com.stagnationlab.c8y.driver.controllers.TrainController;
import com.stagnationlab.c8y.driver.controllers.TruckController;
import com.stagnationlab.c8y.driver.controllers.WeatherController;
import com.stagnationlab.c8y.driver.devices.AbstractDevice;
import com.stagnationlab.c8y.driver.fragments.Controller;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.c8y.driver.services.TextToSpeech;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.SocketClient;

import c8y.lx.driver.OperationExecutor;

@SuppressWarnings({ "SameParameterValue" })
@Slf4j
public class Gateway extends AbstractDevice {

	private static final String VERSION = "1.27.0";

	private final com.stagnationlab.c8y.driver.fragments.Gateway state = new com.stagnationlab.c8y.driver.fragments.Gateway();
	private final Config config = new Config();
	private final Map<String, Commander> commanders = new HashMap<>();
	private final EventBroker eventBroker = new EventBroker();

	private static final String CONFIG_FILENAME = "config.properties";

	public Gateway() {
		super("Gateway");

		state.setVersion(VERSION);
		state.setIsRunning(true);
	}

	@Override
	protected String getType() {
		return state.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return state;
	}

	@Override
	protected void setup() throws Exception {
		log.info("setting up the gateway");

		setupShutdownHook();
		setupConfig();
		setupOperations();
		setupCommanders();
		setupControllers();
	}

	@Override
	public void discoverChildren(ManagedObjectRepresentation parent) {
		super.discoverChildren(parent);

		establishCommanderConnections();
	}

	@Override
	public void start() {
		super.start();

		// update initial state
		updateState(state);
	}

	@Override
	public void shutdown() {
		log.info("shutting down gateway");

		for (String commanderName : commanders.keySet()) {
			Commander commander = commanders.get(commanderName);

			if (commander.isConnected()) {
				log.info("closing commander '{}'", commanderName);

				commander.close();
			} else {
				log.info("commander '{}' is already closed", commanderName);
			}
		}

		state.reset();
		updateState(state);

		super.shutdown();

		log.info("graceful shutdown complete");
	}

	private void establishCommanderConnections() {
		int connectTimeout = config.getInt("socket.connectTimeout");

		log.debug("connecting to commanders (timeout: {})", connectTimeout);

		for (String name : commanders.keySet()) {
			Commander commander = commanders.get(name);

			log.debug("connecting to socket of controller '{}'", name);

			// attempt to connect to the socket (errors will be sent to the listener)
			if (commander.connect(connectTimeout)) {
				log.debug("connected to socket of controller '{}'", name);
			} else {
				log.warn("connecting to socket of controller '{}' failed", name);
			}
		}
	}

	private void setupShutdownHook() {
		Runtime.getRuntime().addShutdownHook(new Thread(this::shutdown, "ShutdownHook"));
	}

	private void setupConfig() {
		String filename = "src/main/resources/" + CONFIG_FILENAME;

		log.info("setting up config from {}", filename);

		InputStream configInputStream = getClass().getClassLoader().getResourceAsStream(CONFIG_FILENAME);

		try {
			config.load(configInputStream);
		} catch (IOException e) {
			throw new RuntimeException("loading configuration from '" + filename + "' failed");
		}
	}

	private void setupCommanders() {
		log.info("setting up commanders");

		List<String> commanderNames = config.getStringArray("commanderNames");

		for (String commanderName : commanderNames) {
			log.info("creating commander {}", commanderName);

			commanders.put(commanderName, createCommander(commanderName));
		}
	}

	private void setupControllers() {
		log.info("setting up controllers");

		registerController(
				new ParkingController("Parking controller", commanders, config, eventBroker)
		);

		registerController(
				new LightingController("Lighting controller", commanders, config, eventBroker)
		);


		registerController(
				new WeatherController("Weather controller", commanders, config, eventBroker)
		);

		registerController(
				new TrainController("Train controller", commanders, config, eventBroker)
		);

		registerController(
				new TruckController("Truck controller", commanders, config, eventBroker)
		);
	}

	private void setupOperations() {
		setupRestartOperation();
	}

	private void setupRestartOperation() {
		registerOperationExecutor(new OperationExecutor() {
			@Override
			public String supportedOperationType() {
				return "c8y_Restart";
			}

			@Override
			public void execute(OperationRepresentation operation, boolean cleanup) throws Exception {
				if (!device.getId().equals(operation.getDeviceId())) {
					return;
				}

				log.info("restarting...");

				operation.setStatus(OperationStatus.SUCCESSFUL.toString());

				new ProcessBuilder(new String[] { "shutdown", "-r" }).start().waitFor();
			}
		});
	}

	private void registerController(AbstractController controller) {
		log.info("registering controller '{}' ({})", controller.getId(), controller.getClass().getSimpleName());

		eventBroker.addListener(controller);

		registerChild(controller);
	}

	private Commander createCommander(String name) {
		int defaultPort = config.getInt("socket.defaultPort");
		int defaultPingInterval = config.getInt("socket.defaultPingInterval");
		int defaultReconnectTimeout = config.getInt("socket.defaultReconnectTimeout");

		String host = config.getString("commander." + name + ".host");
		int port = config.getInt("commander.\" + id + \".port", defaultPort);
		int pingInterval = config.getInt("commander.\" + id + \".pingInterval", defaultPingInterval);
		int reconnectTimeout = config.getInt("commander.\" + id + \".reconnectTimeout", defaultReconnectTimeout);

		log.info("connecting to controller commander '{}' at {}:{}", name, host, port);

		SocketClient socketClient = new SocketClient(host, port, reconnectTimeout);
		Commander commander = new Commander(socketClient);

		// add controller to state
		Controller controller = new Controller(name, host, port, "", Controller.State.UNINITIALIZED);
		state.addController(controller);

		// listen for socket events
		socketClient.addEventListener(new MessageTransport.EventListener() {
			@Override
			public void onConnecting(boolean isReconnecting) {
				if (isReconnecting) {
					log.info("reconnecting to socket of controller '{}' at {}:{}", name, host, port);
				} else {
					log.info("connecting to socket of controller '{}' at {}:{}", name, host, port);
				}

				updateControllerState(name, isReconnecting ? Controller.State.RECONNECTING : Controller.State.CONNECTING);
			}

			@Override
			public void onOpen(boolean isFirstConnect) {
				log.info("socket of controller '{}' at {}:{} was {}, requesting version", name, host, port, isFirstConnect ? "connected" : "reconnected");

				commander.sendCommand("version").thenAccept(commandResponse -> {
					String version = commandResponse.response.getString(0);
					log.info("commander '{}' version: {}", name, version);

					updateControllerVersion(name, version);
				});

				updateControllerState(name, Controller.State.CONNECTED);
			}

			@Override
			public void onClose(boolean isPlanned) {
				if (isPlanned) {
					log.info("socket of controller '{}' at {}:{} was successfully closed", name, host, port);
				} else {
					log.warn("socket of controller '{}' at {}:{} was closed", name, host, port);
				}

				updateControllerState(name, Controller.State.DISCONNECTED);
			}

			@Override
			public void onMessageReceived(String message) {
				// don't log heartbeat messages
				if (message.length() >= 11 && message.substring(0, 11).equals("0:HEARTBEAT")) {
					return;
				}

				log.trace("got controller '{}' message: '{}'", name, message);
			}

			@Override
			public void onConnectionFailed(Exception e, boolean wasEverConnected) {
				if (wasEverConnected) {
					log.debug("reconnecting to controller '{}' at {}:{} failed ({} - {})", name, host, port, e.getClass().getSimpleName(), e.getMessage());

					updateControllerState(name, Controller.State.DISCONNECTED);
				} else {
					log.warn("connecting to controller '{}' at {}:{} failed ({} - {})", name, host, port, e.getClass().getSimpleName(), e.getMessage());

					TextToSpeech.INSTANCE.speak("Connecting to controller \"" + name + "\" failed, some functionality will be disabled");

					updateControllerState(name, Controller.State.CONNECTION_FAILED);
				}
			}
		});

		// set ping strategy to use
		socketClient.setPingStrategy(new SocketClient.PingStrategy() {
			@Override
			public String getPingMessage() {
				return "0:ping\n";
			}

			@Override
			public boolean isPingResponse(String response) {
				return response.equals("0:OK:pong");
			}
		}, pingInterval);

		return commander;
	}

	private void updateControllerState(String name, Controller.State state) {
		if (!isApiReady || device == null) {
			log.warn("updating controller '{}' state to '{}' skipped, api not ready yet", name, state.name());

			return;
		}

		Controller controller = this.state.controllerByName(name);

		if (controller == null) {
			throw new RuntimeException("controller called '{}' not found in gateway state, this should not happen");
		}

		log.debug("updating controller '{}' state to '{}'", name, state.name());

		controller.setState(state);
		updateState(this.state);
	}

	private void updateControllerVersion(String name, String version) {
		if (!isApiReady || device == null) {
			log.warn("updating controller '{}' version to '{}' skipped, api not ready yet", name, version);

			return;
		}

		Controller controller = this.state.controllerByName(name);

		if (controller == null) {
			throw new RuntimeException("controller called '{}' not found in gateway state, this should not happen");
		}

		log.debug("updating controller '{}' version to '{}'", name, version);

		controller.setVersion(version);
		updateState(this.state);
	}
}
