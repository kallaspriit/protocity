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
		OUTPUT,
		INPUT,
		INTERRUPT,
		PWM,
		ANALOG
	};

	enum DigitalValue {
		LOW,
		HIGH
	};

	class PortEventListener {
	public:
		virtual void onPortValueChange(int id, DigitalValue value) {};
		virtual void onPortValueRise(int id) {};
		virtual void onPortValueFall(int id) {};
	};

	PortController(int id, PinName pinName);

	int getId();
	PortMode getMode();
	void setPortMode(PortMode mode);
	void setPinMode(PinMode mode);
	void setValue(DigitalValue value);
	void setValue(int value);
	void setPwmDutyCycle(float dutyCycle);

	static PortMode getPortModeByName(std::string mode);
	static std::string getPortModeName(PortMode mode);

	DigitalValue getDigitalValue();
	float getAnalogValue();
	void addInterruptListener(PortEventListener *listener);

private:
	void handleInterruptRise();
	void handleInterruptFall();

	typedef std::vector<PortEventListener*> ListenerList;
	ListenerList listeners;

	int id;
	PinName pinName;
	PortMode portMode;
	DigitalOut *digitalOut = NULL;
	PwmOut *pwmOut = NULL;
	InterruptIn *interruptIn = NULL;
	DigitalIn *digitalIn = NULL;
	AnalogIn *analogIn = NULL;
	volatile int interruptRiseCount = 0;
	volatile int interruptFallCount = 0;
};

#endif
