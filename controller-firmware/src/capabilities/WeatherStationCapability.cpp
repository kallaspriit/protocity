#include "WeatherStationCapability.hpp"

#include "../PortController.hpp"

#include <string>

WeatherStationCapability::WeatherStationCapability(Serial *serial, PortController *portController, PinName sdaPin, PinName sclPin, PinName txPin, PinName rxPin, PinName resetPin) :
	AbstractCapability(serial, portController),
    sdaPin(sdaPin),
    sclPin(sclPin),
    txPin(txPin),
    rxPin(rxPin),
    resetPin(resetPin)
{}

std::string WeatherStationCapability::getName() {
	return "weather-station";
}

CommandManager::Command::Response WeatherStationCapability::handleCommand(CommandManager::Command *command) {
    if (command->argumentCount < 3) {
        return command->createFailureResponse("no capability action requested");
    }

    std::string action = command->getString(2);

    if (action == "enable") {
        return handleEnableCommand(command);
    } else if (action == "disable") {
        return handleDisableCommand(command);
    } else {
        return command->createFailureResponse("invalid capability action requested");
    }
}

CommandManager::Command::Response WeatherStationCapability::handleEnableCommand(CommandManager::Command *command) {
	if (isEnabled) {
		return command->createSuccessResponse();
	}

	if (!enable()) {
		return command->createFailureResponse("enabling the sensor failed");
	}

	return command->createSuccessResponse();
}

CommandManager::Command::Response WeatherStationCapability::handleDisableCommand(CommandManager::Command *command) {
	if (!isEnabled) {
		return command->createSuccessResponse();
	}

	disable();

	return command->createSuccessResponse();
}

bool WeatherStationCapability::enable() {
	if (isEnabled) {
		return true;
	}

	log.info("enabling weather station");

    // create lcd controller
    restartLcd();

	isEnabled = true;

    // create sensors
    thermometer = new TMP102(sdaPin, sclPin, 0x90);
    lightmeter = new TSL2561(sdaPin, sclPin, TSL2561_ADDR_FLOAT);
    hygrometer = new Si7021(sdaPin, sclPin);
    barometer = new MPL3115A2(sdaPin, sclPin, 0x60 << 1);
    soundmeter = new AnalogIn(portController->getPinName());

	// configure lightmeter
	//lightmeter->setGain(TSL2561_GAIN_0X);
	//lightmeter->setTiming(TSL2561_INTEGRATIONTIME_402MS);
	lightmeter->setGain(TSL2561_GAIN_16X);
	lightmeter->setTiming(TSL2561_INTEGRATIONTIME_13MS);

    // configure barometer
    barometer->Oversample_Ratio(OVERSAMPLE_RATIO_32);
	barometer->Barometric_Mode();

	// start timers
    thermometerTimer.start();
    lightmeterTimer.start();
    hygrometerTimer.start();
    barometerTimer.start();
    soundmeterTimer.start();

	// clap detection
	silentTimer.start();
	resetSilentLoudPattern();

	// start threads
	soundThread.start(callback(this, &WeatherStationCapability::runSoundThread));
	updateThread.start(callback(this, &WeatherStationCapability::runUpdateThread));

	return true;
}

void WeatherStationCapability::disable() {
	if (!isEnabled) {
		return;
	}

	log.info("disabling weather station");

	isEnabled = false;

	delete lcd;
	lcd = NULL;

    delete thermometer;
	thermometer = NULL;

    delete lightmeter;
	lightmeter = NULL;

    delete hygrometer;
	hygrometer = NULL;

    delete barometer;
	barometer = NULL;

    delete soundmeter;
	soundmeter = NULL;

    thermometerTimer.stop();
    lightmeterTimer.stop();
    hygrometerTimer.stop();
    barometerTimer.stop();
    soundmeterTimer.stop();
}

void WeatherStationCapability::update(int deltaUs) {
	// we're using our own thread not to interrupt the main thread
}

void WeatherStationCapability::runUpdateThread() {
	log.debug("starting update thread");

	while (isEnabled) {
		if (lcd->hasDied()) {
			log.warn("seems that the LCD has died");

			restartLcd();
		}

		updateReadings();
	}

	log.debug("update thread finished");
}

void WeatherStationCapability::runSoundThread() {
	log.debug("starting sound thread");

	while (isEnabled) {
		updateClapDetection();
	}

	log.debug("sound thread finished");
}

void WeatherStationCapability::updateReadings() {
    if (updateThermometerReading()) {
        sendMeasurement("thermometer", thermometerLastRenderedValue);
    }

    if (updateLightmeterReading()) {
        sendMeasurement("lightmeter", lightmeterLastRenderedValue);
    }

    if (updateHygrometerReading()) {
        sendMeasurement("hygrometer", hygrometerLastRenderedValue);
    }

    if (updateBarometerReading()) {
        sendMeasurement("barometer", barometerLastRenderedValue);
    }

    if (updateSoundmeterReading()) {
        sendMeasurement("soundmeter", soundmeterLastRenderedValue);
    }
}

void WeatherStationCapability::updateClapDetection() {
	if (!isEnabled || soundmeter == NULL) {
		return;
	}

	soundLevel = soundmeter->read() * 100.0f;

	if (soundLevel >= SOUNDMETER_LOUD_THRESHOLD && !wasLoud) {
		int silentDuration = silentTimer.read_ms();

		silentTimer.reset();
		silentTimer.stop();
		loudTimer.start();

		wasLoud = true;
		isSoundPatternActive = true;

		if (silentLoudPatternCount < SOUNDMETER_PATTERN_LENGTH) {
			silentLoudPattern[silentLoudPatternCount++] = silentDuration;
		}

		log.info("got loud sound after %d ms of silence (%f dB)", silentDuration, soundLevel);
	} else if (soundLevel < SOUNDMETER_SILENT_THRESHOLD) {
		if (wasLoud) {
			int loudDuration = loudTimer.read_ms();

			loudTimer.reset();
			loudTimer.stop();
			silentTimer.start();

			wasLoud = false;

			if (silentLoudPatternCount < SOUNDMETER_PATTERN_LENGTH) {
				silentLoudPattern[silentLoudPatternCount++] = loudDuration;
			}

			log.info("got silence after %d ms of loudness (%f dB)", loudDuration, soundLevel);
		} else if (isSoundPatternActive) {
			int silentDuration = silentTimer.read_ms();

			if (silentDuration >= SOUNDMETER_SILENCE_DECISION_THRESHOLD) {
				log.debug("make pattern decision after %d ms with %d values (%f dB)", silentDuration, silentLoudPatternCount, soundLevel);

				// expecting even number of datapoints
				if (silentLoudPatternCount % 2 == 0) {
					bool wereAllClaps = true;
					int clapCount = 0;

					// detect claps
					for (int i = 0; i < silentLoudPatternCount; i += 2) {
						int silenceDuration = silentLoudPattern[i];
						int loudDuration = silentLoudPattern[i + 1];
						bool isClap = loudDuration <= SOUNDMETER_CLAP_LOUD_THRESHOLD && (i == 0 || silenceDuration <= SOUNDMETER_CLAP_SILENT_THRESHOLD);

						if (isClap) {
							clapCount++;
						} else {
							wereAllClaps = false;
						}

						log.debug("  silence for %dms followed by loud sound for %d ms, %s", silenceDuration, loudDuration, isClap ? "detected as a clap" : "not a clap");
					}

					if (wereAllClaps && clapCount > 0) {
						log.info("detected claps: %d", clapCount);
					} else {
						log.debug("no continuous claps detected");
					}
				} else {
					log.warn("got loud pattern with uneven number of datapoints, this should not happen, ignoring it");

					for (int i = 0; i < silentLoudPatternCount; i++) {
						log.warn("  %s for %dms", i % 2 == 0 ? "silence" : "loud", silentLoudPattern[i]);
					}
				}

				isSoundPatternActive = false;
				resetSilentLoudPattern();
			}
		}
	}
}

bool WeatherStationCapability::updateThermometerReading() {
    int timeSinceLastReading = thermometerTimer.read_ms();

    if (thermometerLastRenderedValue > 0 && timeSinceLastReading < thermometerIntervalMs) {
        return false;
    }

    float current = thermometer->read();
    float diff = fabs(current - thermometerLastRenderedValue);
	bool shouldRender = diff >= thermometerRenderChangeThreshold || timeSinceLastReading >= forceUpdateInterval;

	log.trace("read thermometer reading: %f (last: %f, diff: %f, render: %s)", current, thermometerLastRenderedValue, diff, shouldRender ? "yes" : "no");

    if (!shouldRender) {
        return false;
    }

    renderThermometer(current);

    thermometerLastRenderedValue = current;
    thermometerTimer.reset();

    return true;
}

bool WeatherStationCapability::updateLightmeterReading() {
    int timeSinceLastReading = lightmeterTimer.read_ms();

    if (lightmeterLastRenderedValue > 0 && timeSinceLastReading < lightmeterIntervalMs) {
        return false;
    }

    float current = lightmeter->getLuminosity(TSL2561_VISIBLE);
    float diff = fabs(current - lightmeterLastRenderedValue);
	bool shouldRender = diff >= lightmeterRenderChangeThreshold || timeSinceLastReading >= forceUpdateInterval;

	log.trace("read lightmeter reading: %f (last: %f, diff: %f, render: %s)", current, lightmeterLastRenderedValue, diff, shouldRender ? "yes" : "no");

    if (!shouldRender) {
        return false;
    }

	if (current > maximumObservedLightLevel) {
		maximumObservedLightLevel = current;

		log.info("updated maximum observed light level to %f", maximumObservedLightLevel);
	}

    float lightPercentage = fmax(fmin((current / maximumObservedLightLevel) * 100.0f, 100.0f), 0.0f);

	renderLightmeter(current);
    drawProgressBar(0, SIZE_Y - 11, SIZE_X - 1, 10, round(lightPercentage), BLACK, WHITE);

    lightmeterLastRenderedValue = current;
    lightmeterTimer.reset();

    return true;
}

bool WeatherStationCapability::updateHygrometerReading() {
    int timeSinceLastReading = hygrometerTimer.read_ms();

    if (hygrometerLastRenderedValue > 0 && timeSinceLastReading < hygrometerIntervalMs) {
        return false;
    }

    bool wasMeasurementSuccessful = hygrometer->measure();

    if (!wasMeasurementSuccessful) {
        log.warn("measuring humidity failed");

        return false;
    }

    float current = (float)hygrometer->get_humidity() / 1000.0f;
    float diff = fabs(current - hygrometerLastRenderedValue);
	bool shouldRender = diff >= hygrometerRenderChangeThreshold || timeSinceLastReading >= forceUpdateInterval;

	log.trace("read hygrometer reading: %f (last: %f, diff: %f, render: %s)", current, hygrometerLastRenderedValue, diff, shouldRender ? "yes" : "no");

    if (!shouldRender) {
        return false;
    }

    renderHygrometer(current);

    hygrometerLastRenderedValue = current;
    hygrometerTimer.reset();

    return true;
}

bool WeatherStationCapability::updateBarometerReading() {
    int timeSinceLastReading = barometerTimer.read_ms();

    if (barometerLastRenderedValue > 0 && timeSinceLastReading < barometerIntervalMs) {
        return false;
    }

    float current = (barometer->getPressure() / 1000.0f)  / 0.13332239f;
    float diff = fabs(current - barometerLastRenderedValue);
	bool shouldRender = diff >= barometerRenderChangeThreshold || timeSinceLastReading >= forceUpdateInterval;

	log.trace("read barometer reading: %f (last: %f, diff: %f, render: %s)", current, barometerLastRenderedValue, diff, shouldRender ? "yes" : "no");

    if (!shouldRender) {
        return false;
    }

    renderBarometer(current);

    barometerLastRenderedValue = current;
    barometerTimer.reset();

    return true;
}

bool WeatherStationCapability::updateSoundmeterReading() {
	int timeSinceLastReading = soundmeterTimer.read_ms();

	if (soundmeterLastRenderedValue > 0 && timeSinceLastReading < soundmeterIntervalMs) {
        return false;
    }

	// use last sound level from the clap detection thread
	float diff = fabs(soundLevel - soundmeterLastRenderedValue);
	bool shouldRender = diff >= soundmeterRenderChangeThreshold || timeSinceLastReading >= forceUpdateInterval;

	log.trace("read soundmeter reading: %f (last: %f, diff: %f, render: %s)", soundLevel, soundmeterLastRenderedValue, diff, shouldRender ? "yes" : "no");

    if (!shouldRender) {
        return false;
    }

    renderSoundmeter(soundLevel);

    soundmeterLastRenderedValue = soundLevel;
    soundmeterTimer.reset();

    return true;
}

void WeatherStationCapability::resetSilentLoudPattern() {
	isSoundPatternActive = false;
	wasLoud = false;
	silentLoudPatternCount = 0;
	silentTimer.reset();
	silentTimer.start();

	for (int i = 0; i < SOUNDMETER_PATTERN_LENGTH; i++) {
		silentLoudPattern[i] = 0;
	}
}

void WeatherStationCapability::renderBackground() {
	log.debug("rendering LCD background");

    // flip the display
    lcd->display_control(LANDSCAPE_R);

    // draw white rectangle at the top
    lcd->filled_rectangle(0, 0, SIZE_X, 7 * 3, WHITE);

    // use black text on white background
    lcd->color(BLACK);
    lcd->textbackground_color(WHITE);

    // write the title on the rectangle
    lcd->locate(0, 1);
    lcd->printf("  Weather Station");

    // use white text on black background
    lcd->color(WHITE);
    lcd->textbackground_color(BLACK);
}

void WeatherStationCapability::renderThermometer(float value) {
    lcd->locate(0, 4);
    lcd->printf("Temperature  %sC", leftPad(value, 4, 1).c_str());
}

void WeatherStationCapability::renderLightmeter(float value) {
    lcd->locate(0, 6);
    lcd->printf("Luminocity  %slux", leftPad(value, 3, 0).c_str());
}

void WeatherStationCapability::renderHygrometer(float value) {
    lcd->locate(0, 8);
    lcd->printf("Humidity     %s%%", leftPad(value, 4, 1).c_str());
}

void WeatherStationCapability::renderBarometer(float value) {
    lcd->locate(0, 10);
    lcd->printf("Pressure %smmHg", leftPad(value, 5, 1).c_str());
}

void WeatherStationCapability::renderSoundmeter(float value) {
    lcd->locate(0, 12);
    lcd->printf("Sound level  %sdB", leftPad(value, 3, 0).c_str());
}

void WeatherStationCapability::sendMeasurement(std::string name, float value) {
	snprintf(
        sendBuffer,
        SEND_BUFFER_SIZE,
        "%s:%f",
        name.c_str(),
        value
    );

	log.debug("%s measurement changed to %f", name.c_str(), value);

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));
}


std::string WeatherStationCapability::leftPad(float value, int targetLength, int decimals) {
    char valueStr[16];
    char format[16];

    sprintf(format, "%%.%df", decimals);

    std::string result = "";

    int valueLength = sprintf(valueStr, format, value);
    int paddingLength = targetLength - valueLength;

    for (int i = 0; i < paddingLength; i++) {
        result += " ";
    }

    result += valueStr;

    return result;
}

void WeatherStationCapability::drawProgressBar(int x, int y, int width, int height, int percentage, int backgroundColor, int barColor) {
    int innerPadding = 2;
    int innerWidth = ceil((float)(width - innerPadding * 2) * ((float)percentage / 100.0f));

    lcd->rectangle(
        x,
        y,
        x + width,
        y + height,
        barColor
    );
    lcd->filled_rectangle(
        x + innerPadding,
        y + innerPadding,
        x + innerPadding + width - innerPadding * 2,
        y + innerPadding + height - innerPadding * 2,
        backgroundColor
    );
    lcd->filled_rectangle(
        x + innerPadding,
        y + innerPadding,
        x + innerPadding + innerWidth,
        y + innerPadding + height - innerPadding * 2,
        barColor
    );
}

void WeatherStationCapability::restartLcd() {
	if (lcd != NULL) {
		log.debug("restarting lcd, destroying existing lcd controller");

		delete lcd;
		lcd = NULL;
	} else {
		log.info("starting lcd");
	}

	// make all the values render again
	thermometerLastRenderedValue = INVALID_VALUE;
	lightmeterLastRenderedValue = INVALID_VALUE;
	hygrometerLastRenderedValue = INVALID_VALUE;
	barometerLastRenderedValue = INVALID_VALUE;
	soundmeterLastRenderedValue = INVALID_VALUE;

	lcd = new uLCD_4DGL(txPin, rxPin, resetPin);
    lcd->baudrate(BAUD_3000000);

	renderBackground();
}
