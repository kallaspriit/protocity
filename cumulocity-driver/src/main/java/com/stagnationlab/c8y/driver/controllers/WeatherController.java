package com.stagnationlab.c8y.driver.controllers;

import java.util.List;
import java.util.Map;

import lombok.extern.slf4j.Slf4j;

import com.stagnationlab.c8y.driver.constants.ControllerEvent;
import com.stagnationlab.c8y.driver.constants.WeatherSensor;
import com.stagnationlab.c8y.driver.fragments.controllers.Weather;
import com.stagnationlab.c8y.driver.measurements.HumidityMeasurement;
import com.stagnationlab.c8y.driver.measurements.LightMeasurement;
import com.stagnationlab.c8y.driver.measurements.PressureMeasurement;
import com.stagnationlab.c8y.driver.measurements.SoundMeasurement;
import com.stagnationlab.c8y.driver.measurements.TemperatureMeasurement;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.c8y.driver.services.EventBroker;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.MessageTransport;
import com.stagnationlab.etherio.PortController;

@Slf4j
public class WeatherController extends AbstractController {

	private final Weather state = new Weather();
	private Commander commander;
	private PortController portController;
	private boolean isConnected = false;

	private static final String CAPABILITY = "weather-station";

	public WeatherController(String id, Map<String, Commander> commanders, Config config, EventBroker eventBroker) {
		super(id, commanders, config, eventBroker);
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
		log.info("setting up weather controller");

		String commanderName = config.getString("weather.commander");
		int port = config.getInt("weather.port");

		commander = getCommanderByName(commanderName);
		portController = new PortController(port, commander);
	}

	@Override
	public void start() {
		super.start();

		log.info("starting weather controller");

		reportConnectionStatus();

		commander.getMessageTransport().addEventListener(new MessageTransport.EventListener() {

			@Override
			public void onOpen(boolean isFirstConnect) {
				log.info("connection to weather controller commander has been {}", isFirstConnect ? "established" : "re-established");

				if (isFirstConnect) {
					setupEventListener();
				}

				portController.sendPortCommand(CAPABILITY, "enable");

				isConnected = true;

				reportConnectionStatus();
			}

			@Override
			public void onClose(boolean isPlanned) {
				log.info("connection to weather controller commander transport has been closed");

				isConnected = false;

				reportConnectionStatus();
				reportLightLevelEvent(0.0f);
			}
		});
	}

	@Override
	public void shutdown() {
		log.info("shutting down weather controller");

		state.reset();
		updateState(state);

		super.shutdown();
	}

	private void setupEventListener() {
		log.debug("setting up weather event listener");

		portController.addEventListener(new PortController.PortEventListener() {
			@Override
			public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
				if (!capabilityName.equals(CAPABILITY)) {
					return;
				}

				String sensor = arguments.get(0);
				float value = Float.valueOf(arguments.get(1));

				switch (sensor) {
					case WeatherSensor.THERMOMETER:
						handleThermometerUpdate(value);
						break;

					case WeatherSensor.LIGHTMETER:
						handleLightmeterUpdate(value);
						break;

					case WeatherSensor.HYGROMETER:
						handleHygrometerUpdate(value);
						break;

					case WeatherSensor.BAROMETER:
						handleBarometerUpdate(value);
						break;

					case WeatherSensor.SOUNDMETER:
						handleSoundmeterUpdate(value);
						break;
				}
			}
		});
	}

	private void handleThermometerUpdate(float value) {
		reportMeasurement(new TemperatureMeasurement(value));

		state.setTemperature(value);

		updateState(state);
	}

	private void handleLightmeterUpdate(float value) {
		reportMeasurement(new LightMeasurement(value));

		state.setLightLevel(value);

		updateState(state);

		reportLightLevelEvent(value);
	}

	private void handleHygrometerUpdate(float value) {
		reportMeasurement(new HumidityMeasurement(value));

		state.setHumidity(value);

		updateState(state);
	}

	private void handleBarometerUpdate(float value) {
		reportMeasurement(new PressureMeasurement(value));

		state.setPressure(value);

		updateState(state);
	}

	private void handleSoundmeterUpdate(float value) {
		reportMeasurement(new SoundMeasurement(value));

		state.setSoundLevel(value);

		updateState(state);
	}

	private void reportConnectionStatus() {
		state.setIsRunning(isConnected);

		updateState(state);
	}

	private void reportLightLevelEvent(float value) {
		eventBroker.emitEvent(ControllerEvent.LIGHTMETER_CHANGE, value);
	}

}
