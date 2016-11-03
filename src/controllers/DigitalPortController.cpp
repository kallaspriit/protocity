#include "DigitalPortController.hpp"

DigitalPortController::DigitalPortController(int id, PinName pinName) :
	id(id),
 	pinName(pinName),
	portMode(DigitalPortController::PortMode::UNUSED)
{
}

int DigitalPortController::getId() {
	return id;
}

DigitalPortController::PortMode DigitalPortController::getMode() {
	return portMode;
}

void DigitalPortController::setMode(PortMode portMode) {
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
			interruptIn->rise(this, &DigitalPortController::handleInterruptRise);
			interruptIn->fall(this, &DigitalPortController::handleInterruptFall);

			break;

		case PortMode::PWM:
			pwmOut = new PwmOut(pinName);
			break;

		default:
			error("invalid digital port mode %d requested", portMode);
	}
}

void DigitalPortController::setValue(DigitalValue value) {
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

void DigitalPortController::setValue(int value) {
	setValue(value == 0 ? DigitalValue::LOW : DigitalValue::HIGH);
}

void DigitalPortController::setPwmDutyCycle(float dutyCycle) {
	if (dutyCycle < 0.0f || dutyCycle > 1.0f) {
		error("expected duty cycle value between 0.0 and 1.0");
	}

	printf("# setting port %d PWM duty cycle to %f\n", id, dutyCycle);

	*pwmOut = dutyCycle;
}

DigitalPortController::PortMode DigitalPortController::getPortModeByName(std::string mode) {
	if (mode == "OUTPUT") {
		return DigitalPortController::PortMode::OUTPUT;
	} else if (mode == "INPUT") {
		return DigitalPortController::PortMode::INPUT;
	} else if (mode == "INPUT_PULLUP") {
		return DigitalPortController::PortMode::INPUT_PULLUP;
	} else if (mode == "INPUT_PULLDOWN") {
		return DigitalPortController::PortMode::INPUT_PULLDOWN;
	} else if (mode == "INPUT_INTERRUPT") {
		return DigitalPortController::PortMode::INPUT_INTERRUPT;
	} else if (mode == "PWM") {
		return DigitalPortController::PortMode::PWM;
	} else {
		return DigitalPortController::PortMode::INVALID;
	}
}

std::string DigitalPortController::getPortModeName(DigitalPortController::PortMode mode) {
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

DigitalPortController::DigitalValue DigitalPortController::getDigitalValue() {
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

void DigitalPortController::addInterruptListener(DigitalPortController::DigitalPortInterruptListener *listener) {
	printf("# registering interrup listener for port %d\n", id);

	listeners.push_back(listener);
}

void DigitalPortController::handleInterruptRise() {
	interruptRiseCount++;

	for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)->onDigitalPortRise(id);
		(*it)->onDigitalPortChange(id, DigitalValue::HIGH);
	}
}

void DigitalPortController::handleInterruptFall() {
	interruptFallCount++;

	for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); ++it) {
		(*it)->onDigitalPortFall(id);
		(*it)->onDigitalPortChange(id, DigitalValue::LOW);
	}
}
