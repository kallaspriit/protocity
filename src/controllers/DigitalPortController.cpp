#include "DigitalPortController.hpp"

DigitalPortController::DigitalPortController(int id, PinName pinName) :
	id(id),
 	pinName(pinName),
	digitalOut(pinName)
{

}

void DigitalPortController::setMode(PortMode mode) {

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

}

DigitalPortController::DigitalValue DigitalPortController::getDigitalReading() {
	return DigitalValue::LOW; // TODO implement
}

void DigitalPortController::addChangeListener(DigitalPortController::DigitalPortChangeListener *listener) {

}

void DigitalPortController::addIntervalListener(DigitalPortController::DigitalPortChangeListener *listener, int intervalMs) {

}
