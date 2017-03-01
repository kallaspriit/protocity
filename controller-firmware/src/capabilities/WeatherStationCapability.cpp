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
{
    DigitalOut reset(resetPin);

    reset = 0;
    wait_ms(5);
    reset = 1;
}

std::string WeatherStationCapability::getName() {
	return "weather-station";
}

void WeatherStationCapability::update(int deltaUs) {
	// we're using our own thread not to interrupt the main thread
}

void WeatherStationCapability::runUpdateThread() {
	while (isEnabled) {
		updateLcd();

		Thread::wait(renderInterval);
	}
}

void WeatherStationCapability::updateLcd() {
    if (lcd == NULL) {
        return;
    }

    int timeSinceLastRender = renderTimer.read_ms();

    if (timeSinceLastRender < renderInterval) {
        return;
    }

    updateReadings();

    renderTimer.reset();
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

bool WeatherStationCapability::updateThermometerReading() {
    int timeSinceLastReading = thermometerTimer.read_ms();

    if (thermometerLastRenderedValue > 0 && timeSinceLastReading < thermometerIntervalMs) {
        return false;
    }

    float current = thermometer->read();
    float diff = fabs(current - thermometerLastRenderedValue);

    if (diff < thermometerRenderChangeThreshold && timeSinceLastReading < forceUpdateInterval) {
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

    if (diff < lightmeterRenderChangeThreshold && timeSinceLastReading < forceUpdateInterval) {
        return false;
    }

    renderLightmeter(current);

    float maxLightValue = 300.0f;
    float lightPercentage = fmax(fmin((current / maxLightValue) * 100.0f, 100.0f), 0.0f);
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
        printf("# measuring humidity failed\n");

        return false;
    }

    float current = (float)hygrometer->get_humidity() / 1000.0f;
    float diff = fabs(current - hygrometerLastRenderedValue);

    if (diff < hygrometerRenderChangeThreshold && timeSinceLastReading < forceUpdateInterval) {
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

    if (diff < barometerRenderChangeThreshold && timeSinceLastReading < forceUpdateInterval) {
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

    float current = soundmeter->read() * 100.0f;
    float diff = fabs(current - soundmeterLastRenderedValue);

    if (diff < soundmeterRenderChangeThreshold && timeSinceLastReading < forceUpdateInterval) {
        return false;
    }

    renderSoundmeter(current);

    soundmeterLastRenderedValue = current;
    soundmeterTimer.reset();

    return true;
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

	printf("# enabling weather station\n");

    // create lcd controller
    lcd = new uLCD_4DGL(txPin, rxPin, resetPin);
    lcd->baudrate(BAUD_3000000);

    // create sensors
    thermometer = new TMP102(sdaPin, sclPin, 0x90);
    lightmeter = new TSL2561(sdaPin, sclPin, TSL2561_ADDR_FLOAT);
    hygrometer = new Si7021(sdaPin, sclPin);
    barometer = new MPL3115A2(sdaPin, sclPin, 0x60 << 1);
    soundmeter = new AnalogIn(portController->getPinName());

	// configure lightmeter
	lightmeter->setGain(TSL2561_GAIN_0X);
	lightmeter->setTiming(TSL2561_INTEGRATIONTIME_402MS);

    // configure barometer
    barometer->Oversample_Ratio(OVERSAMPLE_RATIO_32);
	barometer->Barometric_Mode();

    renderBackground();

    renderTimer.start();
    thermometerTimer.start();
    lightmeterTimer.start();
    hygrometerTimer.start();
    barometerTimer.start();
    soundmeterTimer.start();

	isEnabled = true;

	updateThread.start(this, &WeatherStationCapability::runUpdateThread);

	return true;
}

void WeatherStationCapability::disable() {
	if (!isEnabled) {
		return;
	}

	printf("# disabling weather station\n");

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

    renderTimer.stop();
    thermometerTimer.stop();
    lightmeterTimer.stop();
    hygrometerTimer.stop();
    barometerTimer.stop();
    soundmeterTimer.stop();

	isEnabled = false;
}

void WeatherStationCapability::renderBackground() {
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
