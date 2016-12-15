#include "mbed.h"
#include "rtos.h"

#include <sdram.h>

#include "Application.hpp"

int main() {
	if (sdram_init() != 0) {
        error("failed to initialize SDRAM\n");
    }

	Serial serial(USBTX, USBRX);
	serial.baud(115200);

	printf("### NFC testing ###\n");

	Application application(&serial);

	application.run();
}
