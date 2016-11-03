#ifndef DIGITALPORTCONTROLLER_HPP
#define DIGITALPORTCONTROLLER_HPP

#include "mbed.h"

#include "../AbstractController.hpp"

#include <string>
#include <vector>

class DigitalPortController : public AbstractController {

public:
	enum PortMode {
		UNUSED,
		INVALID,
		OUTPUT,
		INPUT,
		INPUT_PULLUP,
		INPUT_PULLDOWN,
		INPUT_INTERRUPT,
		PWM
	};

	enum DigitalValue {
		LOW,
		HIGH
	};

	class DigitalPortInterruptListener {
	public:
		virtual void onDigitalPortChange(int id, DigitalValue value) {};
		virtual void onDigitalPortRise(int id) {};
		virtual void onDigitalPortFall(int id) {};
	};

	DigitalPortController(int id, PinName pinName);

	int getId();
	PortMode getMode();
	void setMode(PortMode mode);
	void setValue(DigitalValue value);
	void setValue(int value);
	void setPwmDutyCycle(float dutyCycle);

	static PortMode getPortModeByName(std::string mode);
	static std::string getPortModeName(PortMode mode);

	DigitalValue getDigitalValue();
	void addInterruptListener(DigitalPortInterruptListener *listener);

private:
	void handleInterruptRise();
	void handleInterruptFall();

	typedef std::vector<DigitalPortInterruptListener*> ListenerList;
	ListenerList listeners;

	int id;
	PinName pinName;
	PortMode portMode;
	DigitalOut *digitalOut = NULL;
	PwmOut *pwmOut = NULL;
	InterruptIn *interruptIn = NULL;
	DigitalIn *digitalIn = NULL;
	volatile int interruptRiseCount = 0;
	volatile int interruptFallCount = 0;
};

#endif
