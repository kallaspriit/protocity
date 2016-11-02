#include "DigitalPortController.hpp"

DigitalPortController::DigitalPortController(int id, PinName pinName, DigitalPortController::PortMode portMode) :
	id(id),
 	pinName(pinName),
	digitalOut(pinName),
	digitalIn(pinName),
	pwmOut(pinName)
{
	setMode(portMode);
}

int DigitalPortController::getId() {
	return id;
}

void DigitalPortController::setMode(PortMode portMode) {
	printf("# setting mode %d for port %d", portMode, id);

	this->portMode = portMode;

	switch (portMode) {
		case PortMode::OUTPUT:
			digitalOut = DigitalOut(pinName);
			break;

		case PortMode::INPUT:
			digitalIn = DigitalIn(pinName);
			break;

		case PortMode::PWM:
			pwmOut = PwmOut(pinName);
			break;

		default:
			error("invalid digital port mode %d requested", portMode);
	}
}

void DigitalPortController::setValue(DigitalValue value) {
	switch (value) {
		case DigitalValue::LOW:
			digitalOut = 0;
			break;

		case DigitalValue::HIGH:
			digitalOut = 1;
			break;

		default:
			error("expected either DigitalValue::LOW (0) or DigitalValue::HIGH (1), got %d", value);
	}
}

void DigitalPortController::setPwmDutyCycle(float dutyCycle) {
	if (dutyCycle < 0.0f || dutyCycle > 1.0f) {
		error("expected duty cycle value between 0.0 and 1.0");
	}

	pwmOut = dutyCycle;
}

DigitalPortController::DigitalValue DigitalPortController::getDigitalReading() {
	return DigitalValue::LOW; // TODO implement
}

void DigitalPortController::addChangeListener(DigitalPortController::DigitalPortChangeListener *listener) {

}

void DigitalPortController::addIntervalListener(DigitalPortController::DigitalPortChangeListener *listener, int intervalMs) {

}
