#ifndef DIGITALPORTCONTROLLER_HPP
#define DIGITALPORTCONTROLLER_HPP

#include "mbed.h"

#include "../AbstractController.hpp"

#include <string>
#include <vector>

class PortController : public AbstractController {

public:
	enum PortMode {
		UNUSED,
		INVALID,
		DIGITAL_OUT,
		DIGITAL_IN,
		ANALOG_OUT,
		ANALOG_IN,
		INTERRUPT
	};

	enum DigitalValue {
		LOW,
		HIGH
	};

	class PortEventListener {

	public:
		virtual void onPortDigitalValueChange(int id, DigitalValue value) {};
		virtual void onPortAnalogValueChange(int id, float value) {};
		virtual void onPortValueRise(int id) {};
		virtual void onPortValueFall(int id) {};
	};

	PortController(int id, PinName pinName);

	int getId();
	PortMode getPortMode();
	void setPortMode(PortMode mode);
	void setPinMode(PinMode mode);
	static PortMode getPortModeByName(std::string mode);
	static std::string getPortModeName(PortMode mode);

	DigitalValue getDigitalValue();
	void setDigitalValue(DigitalValue value);
	void setDigitalValue(int value);

	float getAnalogValue();
	void setAnalogValue(float dutyCycle);

	void addEventListener(PortEventListener *listener);
	void addEventListener(PortEventListener *listener, float changeThreshold, int intervalMs);
	void listenAnalogValueChange(float changeThreshold, int intervalMs);
	void stopAnalogValueListener();

	void update(int deltaUs);

private:
	void handleInterruptRise();
	void handleInterruptFall();

	void updateValueChange(PortEventListener* listener);
	void emitAnalogValueChangeEvent(PortEventListener* listener, float value);

	// configuration
	int id;
	PinName pinName;
	PortMode portMode;

	// listeners
	typedef std::vector<PortEventListener*> ListenerList;
	ListenerList listeners;

	// pin modes
	DigitalOut *digitalOut = NULL;
	PwmOut *pwmOut = NULL;
	InterruptIn *interruptIn = NULL;
	DigitalIn *digitalIn = NULL;
	AnalogIn *analogIn = NULL;

	// interrupt
	volatile int interruptRiseCount = 0;
	volatile int interruptFallCount = 0;

	// analog port listeners
	float analogChangeEventThreshold = 0.0f;
	float analogChangeEventLastValue = 0.0f;
	int analogChangeEventIntervalUs = 0;
	int analogChangeEventUsSinceLastUpdate = 0;
	bool isAnalogChangeEventEnabled = false;
};

#endif
