#ifndef WEATHERSTATIONCAPABILITY_HPP
#define WEATHERSTATIONCAPABILITY_HPP

#include "../AbstractCapability.hpp"
#include "Log.hpp"

#include <uLCD_4DGL.hpp>
#include <MPL3115A2.hpp>
#include <Si7021.hpp>
#include <TMP102.hpp>
#include <TSL2561.hpp>

class WeatherStationCapability : public AbstractCapability {

public:
	WeatherStationCapability(Serial *serial, PortController *portController, PinName sdaPin = p9, PinName sclPin = p10, PinName txPin = p37, PinName rxPin = p31, PinName resetPin = p8);

	std::string getName();
	void update(int deltaUs);

	CommandManager::Command::Response handleCommand(CommandManager::Command *command);

private:
	Log log = Log::getLog("WeatherStationCapability");

	CommandManager::Command::Response handleEnableCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDisableCommand(CommandManager::Command *command);

	bool enable();
	void disable();
	void sendMeasurement(std::string name, float value);

	void renderBackground();
	void renderThermometer(float value);
	void renderLightmeter(float value);
	void renderHygrometer(float value);
	void renderBarometer(float value);
	void renderSoundmeter(float value);

	void runUpdateThread();
	void restartLcd();
	void updateReadings();
	bool updateThermometerReading();
	bool updateLightmeterReading();
	bool updateHygrometerReading();
	bool updateBarometerReading();
	bool updateSoundmeterReading();

	std::string leftPad(float value, int targetLength, int decimals = 1);
	void drawProgressBar(int x, int y, int width, int height, int percentage, int backgroundColor, int barColor);

	PinName sdaPin;
    PinName sclPin;
    PinName txPin;
    PinName rxPin;
    PinName resetPin;

    uLCD_4DGL *lcd = NULL;
	TMP102 *thermometer = NULL;
	TSL2561 *lightmeter = NULL;
	Si7021 *hygrometer = NULL;
	MPL3115A2 *barometer = NULL;
	AnalogIn *soundmeter = NULL;

	bool isEnabled = false;

	Thread updateThread;

	Timer thermometerTimer;
	Timer lightmeterTimer;
	Timer hygrometerTimer;
	Timer barometerTimer;
	Timer soundmeterTimer;

	const float INVALID_VALUE = -100.0f;

	int updateFps = 10;
	int updateInterval = 1000 / updateFps;

	float thermometerLastRenderedValue = INVALID_VALUE;
	float thermometerRenderChangeThreshold = 0.01f;
	int thermometerIntervalMs = 1000;

	float lightmeterLastRenderedValue = INVALID_VALUE;
	float lightmeterRenderChangeThreshold = 5.0f;
	int lightmeterIntervalMs = 1000;
	float maximumObservedLightLevel = 300.0f;

	float hygrometerLastRenderedValue = INVALID_VALUE;
	float hygrometerRenderChangeThreshold = 0.1f;
	int hygrometerIntervalMs = 10000;

	float barometerLastRenderedValue = INVALID_VALUE;
	float barometerRenderChangeThreshold = 0.1f;
	int barometerIntervalMs = 10000;

	float soundmeterLastRenderedValue = INVALID_VALUE;
	float soundmeterRenderChangeThreshold = 5.0f;
	int soundmeterIntervalMs = 1000;

	float forceUpdateInterval = 60000;

};

#endif
