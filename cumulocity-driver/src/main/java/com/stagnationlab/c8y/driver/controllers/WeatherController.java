package com.stagnationlab.c8y.driver.controllers;

import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.stagnationlab.c8y.driver.measurements.HumidityMeasurement;
import com.stagnationlab.c8y.driver.measurements.LightMeasurement;
import com.stagnationlab.c8y.driver.measurements.PressureMeasurement;
import com.stagnationlab.c8y.driver.measurements.SoundMeasurement;
import com.stagnationlab.c8y.driver.measurements.TemperatureMeasurement;
import com.stagnationlab.c8y.driver.services.Config;
import com.stagnationlab.etherio.Commander;
import com.stagnationlab.etherio.PortController;

public class WeatherController extends AbstractController {

	private static final Logger log = LoggerFactory.getLogger(WeatherController.class);

	private final com.stagnationlab.c8y.driver.fragments.WeatherController state = new com.stagnationlab.c8y.driver.fragments.WeatherController();
	private PortController portController;

	private float lastTemperature = 0.0f;
	private float lastLightLevel = 0.0f;
	private float lastHumidity = 0.0f;
	private float lastPressure = 0.0f;
	private float lastSoundLevel = 0.0f;

	private static final String CAPABILITY = "weather-station";

	public WeatherController(String id, Map<String, Commander> commanders, Config config) {
		super(id, commanders, config);
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

		Commander commander = commanders.get(commanderName);
		portController = new PortController(port, commander);
	}

	@Override
	public void start() {
		super.start();

		log.info("starting weather controller");

		portController.sendPortCommand(CAPABILITY, "enable");

		portController.addEventListener(new PortController.PortEventListener() {
			@Override
			public void onPortCapabilityUpdate(int id, String capabilityName, List<String> arguments) {
				if (!capabilityName.equals(CAPABILITY)) {
					return;
				}

				float temperature = Float.valueOf(arguments.get(0));
				float lightLevel = Float.valueOf(arguments.get(1));
				float humidity = Float.valueOf(arguments.get(2));
				float pressure = Float.valueOf(arguments.get(3));
				float soundLevel = Float.valueOf(arguments.get(4));

				handleWeatherUpdate(temperature, lightLevel, humidity, pressure, soundLevel);
			}
		});
	}

	private void handleWeatherUpdate(float temperature, float lightLevel, float humidity, float pressure, float soundLevel) {
		log.debug(
				"weather update temperature: {}C, light level: {} lux, humidity: {}%, pressure: {}mmHg, sound level: {}dB",
				temperature,
				lightLevel,
				humidity,
				pressure,
				soundLevel
		);

		if (temperature != lastTemperature) {
			reportMeasurement(new TemperatureMeasurement(temperature));

			lastTemperature = temperature;
		}

		if (lightLevel != lastLightLevel) {
			reportMeasurement(new LightMeasurement(lightLevel));

			lastLightLevel = lightLevel;
		}

		if (humidity != lastHumidity) {
			reportMeasurement(new HumidityMeasurement(humidity));

			lastHumidity = humidity;
		}

		if (pressure != lastPressure) {
			reportMeasurement(new PressureMeasurement(pressure));

			lastPressure = pressure;
		}

		if (soundLevel != lastSoundLevel) {
			reportMeasurement(new SoundMeasurement(soundLevel));

			lastSoundLevel = soundLevel;
		}

		state.setTemperature(temperature);
		state.setLightLevel(lightLevel);
		state.setHumidity(humidity);
		state.setPressure(pressure);
		state.setSoundLevel(soundLevel);

		updateState(state);
	}

}
