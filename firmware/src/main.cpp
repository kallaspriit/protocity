#include "mbed.h"

#include "Config.hpp"
#include "Application.hpp"

int main() {
	Config config;

	Application application(&config);

	application.run();
}
