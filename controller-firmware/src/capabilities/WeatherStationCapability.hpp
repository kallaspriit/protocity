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

	// readings
	void runUpdateThread();
	void clearRenderCache();
	void restartLcd();
	void updateReadings();
	bool updateThermometerReading();
	bool updateLightmeterReading();
	bool updateHygrometerReading();
	bool updateBarometerReading();
	bool updateSoundmeterReading();

	// clap detection
	void runSoundThread();
	void updateClapDetection();
	void resetSilentLoudPattern();
	void scheduleRenderClapCount(int clapCount);
	void renderClapCount(int clapCount);
	void sendClapEvent(int clapCount, float loudestClapLevel);

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
	Thread soundThread;

	Timer thermometerTimer;
	Timer lightmeterTimer;
	Timer hygrometerTimer;
	Timer barometerTimer;
	Timer soundmeterTimer;

	const float INVALID_VALUE = -100.0f;

	float thermometerLastRenderedValue = INVALID_VALUE;
	float thermometerRenderChangeThreshold = 0.0626f;
	int thermometerIntervalMs = 1000;

	float lightmeterLastRenderedValue = INVALID_VALUE;
	float lightmeterRenderChangeThreshold = 10.0f;
	int lightmeterIntervalMs = 14; // 13ms integration time
	float maximumObservedLightLevel = 300.0f;

	float hygrometerLastRenderedValue = INVALID_VALUE;
	float hygrometerRenderChangeThreshold = 0.1f;
	int hygrometerIntervalMs = 10000;

	float barometerLastRenderedValue = INVALID_VALUE;
	float barometerRenderChangeThreshold = 0.1f;
	int barometerIntervalMs = 10000;

	float soundmeterLastRenderedValue = INVALID_VALUE;
	float soundmeterRenderChangeThreshold = 5.0f;
	int soundmeterIntervalMs = 100;
	volatile float soundLevel = 0;

	float forceUpdateInterval = 60000;

	// clap detection config
	const float SOUNDMETER_LOUD_THRESHOLD = 25.0f;
	const float SOUNDMETER_SILENT_THRESHOLD = 20.0f;
	static const int SOUNDMETER_PATTERN_LENGTH = 32;
	static const int SOUNDMETER_CLAP_LOUD_THRESHOLD = 150;
	static const int SOUNDMETER_CLAP_SILENT_THRESHOLD = 500;
	static const int SHOW_CLAP_COUNT_DURATION = 3000;

	// clap detection runtime
	Timer loudTimer;
	Timer silentTimer;
	bool wasLoud = false;
	bool isSoundPatternActive = false;
	int silentLoudPattern[SOUNDMETER_PATTERN_LENGTH];
	int silentLoudPatternCount = 0;
	float loudestClapLevel = 0.0f;
	volatile bool forceSoundLevelRender = false;
	volatile int scheduledClapCount = 0;
};

#endif
