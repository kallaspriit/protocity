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

PortController::PortMode PortController::getMode() {
	return portMode;
}

void PortController::setMode(PortMode portMode) {
	if (portMode == this->portMode) {
		return;
	}

	printf("# setting mode %s for port %d\n", getPortModeName(portMode).c_str(), id);

	this->portMode = portMode;

	if (digitalOut != NULL) delete digitalOut;
	if (pwmOut != NULL) delete pwmOut;
	if (interruptIn != NULL) delete interruptIn;
	if (digitalIn != NULL) delete digitalIn;

	switch (portMode) {
		case PortMode::OUTPUT:
			digitalOut = new DigitalOut(pinName);
			break;

		case PortMode::INPUT:
			digitalIn = new DigitalIn(pinName);
			break;

		case PortMode::INPUT_PULLUP:
			digitalIn = new DigitalIn(pinName);
			digitalIn->mode(PinMode::PullUp);
			break;

		case PortMode::INPUT_PULLDOWN:
			digitalIn = new DigitalIn(pinName);
			digitalIn->mode(PinMode::PullDown);
			break;

		case PortMode::INPUT_INTERRUPT:
			interruptIn = new InterruptIn(pinName);
			interruptIn->rise(this, &PortController::handleInterruptRise);
			interruptIn->fall(this, &PortController::handleInterruptFall);

			break;

		case PortMode::PWM:
			pwmOut = new PwmOut(pinName);
			break;

		default:
			error("invalid digital port mode %d requested", portMode);
	}
}

void PortController::setValue(DigitalValue value) {
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

void PortController::setValue(int value) {
	setValue(value == 0 ? DigitalValue::LOW : DigitalValue::HIGH);
}

void PortController::setPwmDutyCycle(float dutyCycle) {
	if (dutyCycle < 0.0f || dutyCycle > 1.0f) {
		error("expected duty cycle value between 0.0 and 1.0");
	}

	printf("# setting port %d PWM duty cycle to %f\n", id, dutyCycle);

	*pwmOut = dutyCycle;
}

PortController::PortMode PortController::getPortModeByName(std::string mode) {
	if (mode == "OUTPUT") {
		return PortController::PortMode::OUTPUT;
	} else if (mode == "INPUT") {
		return PortController::PortMode::INPUT;
	} else if (mode == "INPUT_PULLUP") {
		return PortController::PortMode::INPUT_PULLUP;
	} else if (mode == "INPUT_PULLDOWN") {
		return PortController::PortMode::INPUT_PULLDOWN;
	} else if (mode == "INPUT_INTERRUPT") {
		return PortController::PortMode::INPUT_INTERRUPT;
	} else if (mode == "PWM") {
		return PortController::PortMode::PWM;
	} else {
		return PortController::PortMode::INVALID;
	}
}

std::string PortController::getPortModeName(PortController::PortMode mode) {
	switch (mode) {
		case PortMode::UNUSED:
			return "UNUSED";

		case PortMode::INVALID:
			return "INVALID";

		case PortMode::OUTPUT:
			return "OUTPUT";

		case PortMode::INPUT:
			return "INPUT";

		case PortMode::INPUT_PULLUP:
			return "INPUT_PULLUP";

		case PortMode::INPUT_PULLDOWN:
			return "INPUT_PULLDOWN";

		case PortMode::INPUT_INTERRUPT:
			return "INPUT_INTERRUPT";

		case PortMode::PWM:
			return "PWM";

		default:
			return "INVALID";
	}
}

PortController::DigitalValue PortController::getDigitalValue() {
	int value = 0;

	if (portMode == PortMode::INPUT_INTERRUPT) {
		value = interruptIn->read();
	} else if (
		portMode == PortMode::INPUT
		|| portMode == PortMode::INPUT_PULLUP
		|| portMode == PortMode::INPUT_PULLDOWN
	) {
		value = digitalIn->read();
	} else {
		printf("# getting digital reading is valid only for port configured as input\n");

		return DigitalValue::LOW;
	}

	printf("# digital value of port %d: %d, rise count: %d, fall count: %d\n", id, value, interruptRiseCount, interruptFallCount);

	return value == 1 ? DigitalValue::HIGH : DigitalValue::LOW;
}

void PortController::addInterruptListener(PortController::PortEventListener *listener) {
	printf("# registering interrup listener for port %d\n", id);

	listeners.push_back(listener);
}

void PortController::handleInterruptRise() {
	interruptRiseCount++;

	for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)->onPortValueRise(id);
		(*it)->onPortValueChange(id, DigitalValue::HIGH);
	}
}

void PortController::handleInterruptFall() {
	interruptFallCount++;

	for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)->onPortValueFall(id);
		(*it)->onPortValueChange(id, DigitalValue::LOW);
	}
}
