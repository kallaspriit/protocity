#ifndef ABSTRACTCONTROLLER_HPP
#define ABSTRACTCONTROLLER_HPP

#include "mbed.h"

class AbstractController {

public:
	virtual void update(int deltaUs) {};

};

#endif
