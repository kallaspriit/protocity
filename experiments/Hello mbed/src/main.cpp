#include "mbed.h"
#include "rtos.h"
//#include <sdram.h>

Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);
DigitalOut led2(LED2);

int main() {
	pc.baud(115200);

	//if (sdram_init() != 0) {
    //    printf("failed to initialize SDRAM\n");
    //}

	led1 = 0;
	led2 = 1;

	while (true) {
		pc.printf("hey!\n");

		led1 = !led1;
		led2 = !led2;

		Thread::wait(1000);
	}
}
