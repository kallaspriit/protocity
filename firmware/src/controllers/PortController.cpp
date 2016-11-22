#include "PortController.hpp"

PortController::PortController(int id, PinName pinName) :
	id(id),
 	pinName(pinName),
	portMode(PortController::PortMode::UNUSED)
{
}

int PortController::getId() {
	return id;
}

PortController::PortMode PortController::getPortMode() {
	return portMode;
}

void PortController::setPortMode(PortMode portMode) {
	if (portMode == this->portMode) {
		return;
	}

	printf("# setting mode %s for port %d\n", getPortModeName(portMode).c_str(), id);

	this->portMode = portMode;

	if (digitalOut != NULL) delete digitalOut;
	if (pwmOut != NULL) delete pwmOut;
	if (interruptIn != NULL) delete interruptIn;
	if (digitalIn != NULL) delete digitalIn;
	if (analogIn != NULL) delete analogIn;

	switch (portMode) {
		case PortMode::UNUSED:
			// dont do anything
			break;

		case PortMode::OUTPUT:
			digitalOut = new DigitalOut(pinName);
			break;

		case PortMode::INPUT:
			digitalIn = new DigitalIn(pinName);
			break;

		case PortMode::INTERRUPT:
			interruptIn = new InterruptIn(pinName);
			interruptIn->rise(this, &PortController::handleInterruptRise);
			interruptIn->fall(this, &PortController::handleInterruptFall);

			break;

		case PortMode::PWM:
			pwmOut = new PwmOut(pinName);
			break;

		case PortMode::ANALOG:
			analogIn = new AnalogIn(pinName);
			break;

		default:
			error("invalid port mode %d requested", portMode);
	}
}

void PortController::setPinMode(PinMode pinMode) {
	switch (portMode) {
		case PortMode::INPUT:
			digitalIn->mode(pinMode);
			break;

		case PortMode::INTERRUPT:
			interruptIn->mode(pinMode);
			break;

		default:
			error("setting pin mode is only valid for INPUT and INTERRUPT ports");
	}
}

void PortController::setDigitalValue(DigitalValue value) {
	switch (value) {
		case DigitalValue::LOW:
			printf("# setting port %d to digital LOW\n", id);

			*digitalOut = 0;
			break;

		case DigitalValue::HIGH:
			printf("# setting port %d to digital HIGH\n", id);

			*digitalOut = 1;
			break;

		default:
			error("expected either DigitalValue::LOW (0) or DigitalValue::HIGH (1), got %d", value);
	}
}

void PortController::setDigitalValue(int value) {
	setDigitalValue(value == 0 ? DigitalValue::LOW : DigitalValue::HIGH);
}

void PortController::setAnalogValue(float dutyCycle) {
	if (dutyCycle < 0.0f || dutyCycle > 1.0f) {
		error("expected duty cycle value between 0.0 and 1.0");
	}

	printf("# setting port %d PWM duty cycle to %f\n", id, dutyCycle);

	*pwmOut = dutyCycle;
}

PortController::PortMode PortController::getPortModeByName(std::string mode) {
	if (mode == "UNUSED") {
		return PortController::PortMode::UNUSED;
	}if (mode == "OUTPUT") {
		return PortController::PortMode::OUTPUT;
	} else if (mode == "INPUT") {
		return PortController::PortMode::INPUT;
	} else if (mode == "INTERRUPT") {
		return PortController::PortMode::INTERRUPT;
	} else if (mode == "PWM") {
		return PortController::PortMode::PWM;
	} else if (mode == "ANALOG") {
		return PortController::PortMode::ANALOG;
	} else {
		return PortController::PortMode::INVALID;
	}
}

std::string PortController::getPortModeName(PortController::PortMode mode) {
	switch (mode) {
		case PortMode::UNUSED:
			return "UNUSED";

		case PortMode::OUTPUT:
			return "OUTPUT";

		case PortMode::INPUT:
			return "INPUT";

		case PortMode::INTERRUPT:
			return "INTERRUPT";

		case PortMode::PWM:
			return "PWM";

		case PortMode::ANALOG:
			return "ANALOG";

		case PortMode::INVALID:
			return "INVALID";

		default:
			return "INVALID";
	}
}

PortController::DigitalValue PortController::getDigitalValue() {
	int value = 0;

	if (portMode == PortMode::INTERRUPT) {
		value = interruptIn->read();
	} else if (portMode == PortMode::INPUT) {
		value = digitalIn->read();
	} else {
		printf("# getting digital reading is valid only for port configured as digital input\n");

		return DigitalValue::LOW;
	}

	printf("# digital value of port %d: %d, rise count: %d, fall count: %d\n", id, value, interruptRiseCount, interruptFallCount);

	return value == 1 ? DigitalValue::HIGH : DigitalValue::LOW;
}

float PortController::getAnalogValue() {
	if (portMode != PortMode::ANALOG) {
		printf("# getting analog reading is valid only for port configured as analog input\n");

		return 0.0f;
	}

	return analogIn->read();
}

void PortController::addEventListener(PortController::PortEventListener *listener) {
	printf("# registering interrup listener for port %d\n", id);

	listeners.push_back(listener);
}

void PortController::addEventListener(PortController::PortEventListener *listener, float changeThreshold, int intervalMs) {
	printf("# registering interrup listener for port %d with change threshold of %f\n", id, changeThreshold);

	listenAnalogValueChange(changeThreshold, intervalMs);

	listeners.push_back(listener);
}

void PortController::listenAnalogValueChange(float changeThreshold, int intervalMs) {
	analogChangeEventThreshold = changeThreshold;
	analogChangeEventIntervalUs = intervalMs * 1000;
	isAnalogChangeEventEnabled = true;
}

void PortController::stopAnalogValueListener() {
	isAnalogChangeEventEnabled = false;
}

void PortController::handleInterruptRise() {
	interruptRiseCount++;

	for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)->onPortValueRise(id);
		(*it)->onPortDigitalValueChange(id, DigitalValue::HIGH);
	}
}

void PortController::handleInterruptFall() {
	interruptFallCount++;

	for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)->onPortValueFall(id);
		(*it)->onPortDigitalValueChange(id, DigitalValue::LOW);
	}
}

void PortController::update(int deltaUs) {
	// the value change events is valid only for analog ports
	if (portMode != PortMode::ANALOG) {
		return;
	}

	int listenerCount = listeners.size();

	// quit early if there are no listeners registered
	if (listenerCount == 0) {
		return;
	}

	if (isAnalogChangeEventEnabled) {
		analogChangeEventUsSinceLastUpdate += deltaUs;
	}

	for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		updateValueChange(*it);
	}
}

void PortController::updateValueChange(PortEventListener* listener) {
	if (!isAnalogChangeEventEnabled) {
		return;
	}

	float currentValue = getAnalogValue();
	float lastValue = analogChangeEventLastValue;
	float delta = currentValue - lastValue;
	bool isDeltaLargeEnough = fabs(delta) >= analogChangeEventThreshold;
	bool isMinMaxReached = (lastValue != 0.0f && currentValue == 0.0f) || (lastValue != 1.0f && currentValue == 1.0f);
	bool hasEnoughtTimePassed = analogChangeEventUsSinceLastUpdate >= analogChangeEventIntervalUs;

	if (hasEnoughtTimePassed && (isDeltaLargeEnough || isMinMaxReached)) {
		emitAnalogValueChangeEvent(listener, currentValue);
	}
}

void PortController::emitAnalogValueChangeEvent(PortEventListener* listener, float value) {
	listener->onPortAnalogValueChange(id, value);

	analogChangeEventLastValue = value;
	analogChangeEventUsSinceLastUpdate = 0;
}
