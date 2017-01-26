package com.stagnationlab.c8y.driver;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.cumulocity.model.operation.OperationStatus;
import com.cumulocity.rest.representation.operation.OperationRepresentation;
import com.stagnationlab.c8y.driver.controllers.ParkingController;
import com.stagnationlab.c8y.driver.devices.AbstractDevice;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.TextToSpeech;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.SocketClient;

import c8y.lx.driver.OperationExecutor;

@SuppressWarnings({ "unused", "SameParameterValue" })
public class Gateway extends AbstractDevice {

	private static final Logger log = LoggerFactory.getLogger(Gateway.class);

	private final com.stagnationlab.c8y.driver.fragments.Gateway gateway = new com.stagnationlab.c8y.driver.fragments.Gateway();
	private final Config config = new Config();
	private final Map<String, Commander> commanders = new HashMap<>();

	private static final int DEFAULT_CONTROLLER_PORT = 8080;
	private static final String CONFIG_FILENAME = "config.properties";

	public Gateway() {
		super("Gateway");
	}

	@Override
	protected String getType() {
		return gateway.getClass().getSimpleName();
	}

	@Override
	protected Object getSensorFragment() {
		return gateway;
	}

	@Override
	protected void setup() throws Exception {
		setupConfig();
		setupCommanders();
		setupControllers();
		setupOperations();
	}

	private void setupConfig() throws IOException {
		log.info("setting up config from src/main/resources/{}", CONFIG_FILENAME);

		InputStream configInputStream = getClass().getClassLoader().getResourceAsStream(CONFIG_FILENAME);

		config.load(configInputStream);
	}

	private void setupCommanders() throws IOException {
		log.info("setting up commanders");

		List<String> commanderNames = config.getStringArray("commandersNames");

		for (String commanderName : commanderNames) {
			log.info("creating commander {}", commanderName);

			commanders.put(commanderName, createCommander(commanderName));
		}
	}

	private void setupControllers() {
		log.info("setting up controllers");

		setupParkingController();
	}

	private void setupParkingController() {
		registerChild(
				new ParkingController("ParkingController controller", commanders, config)
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

	@Override
	public void start() {
		super.start();

		TextToSpeech.speak("Ready, welcome to Telia Lego City!");
	}

	private Commander createCommander(String name) {
		String host = config.getString("commander." + name + ".host");
		int port = config.getInt("commander.\" + id + \".port", DEFAULT_CONTROLLER_PORT);

		log.info("connecting to controller commander {} at {}:{}", name, host, port);

		SocketClient socketClient = new SocketClient(host, port);

		socketClient.addMessageListener(new MessageTransport.MessageListener() {
			@Override
			public void onSocketOpen() {
				log.info("socket of controller {} at {}:{} was opened", name, host, port);
			}

			@Override
			public void onSocketClose() {
				log.info("socket of controller {} at {}:{} was closed", name, host, port);
			}

			@Override
			public void onSocketMessageReceived(String message) {
				// don't log heartbeat messages
				if (message.length() >= 11 && message.substring(0, 11).equals("0:HEARTBEAT")) {
					return;
				}

				log.debug("got controller {} message: '{}'", name, message);
			}
		});

		Commander commander = new Commander(socketClient);

		try {
			socketClient.connect();
		} catch (IOException e) {
			log.warn("connecting to controller {} at {}:{} failed ({} - {})", name, host, port, e.getClass().getSimpleName(), e.getMessage());
		}

		commander.sendCommand("version").thenAccept(commandResponse -> log.info("got commander {} version: {}", name, commandResponse.response.getString(0)));

		return commander;
	}
}
