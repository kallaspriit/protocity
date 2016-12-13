#include "mbed.h"
#include "sdram.h"

#define FREEMEM_CELL 1024

struct elem {
    struct elem *next;
    char dummy[FREEMEM_CELL-2];
};

// based on https://developer.mbed.org/questions/6994/How-to-print-Free-RAM-available-RAM-or-u/
int getFreeMemory(void) {
    int counter;
    struct elem *head, *current, *nextone;

    current = head = (struct elem*) malloc(sizeof(struct elem));

    if (head == NULL) {
        return 0;
	}

    counter = 0;

	//__disable_irq();

    do {
        counter++;
        current->next = (struct elem*) malloc(sizeof(struct elem));
        current = current->next;
    } while (current != NULL);

    current = head;

    do {
        nextone = current->next;
        free(current);
        current = nextone;
    } while (nextone != NULL);

	//__enable_irq();

    return counter * FREEMEM_CELL;
}

int main() {
	printf("running memory test!\n");

	if (sdram_init() != 0) {
        printf("failed to initialize SDRAM\n");
    } else {
		printf("SDRAM is initialized\n");
	}

	while (true) {
		int freeMemoryBytes = getFreeMemory();

		printf("> free memory: %d\n", freeMemoryBytes);

		wait(5.0f);
	}
}

/*
#include "mbed.h"
#include "sdram.h"

Serial pc(USBTX, USBRX);                    //Serial USB communications over mbed USB port
DigitalOut led1(LED1);

//--------------------------------------------------------------------------------------------------------------------------------------//
//Test the SDRAM array with pseudo-random data
//memory size = 0x00800000 (8388608 locations * 4 bytes wide = 33.554432 MB

#define DRAMBASEADDR    0xa0000000
#define DRAMSIZE        0x00800000

int testSDRAM(uint32_t seed) {
    volatile uint32_t * const ramBaseAddr = (uint32_t *) DRAMBASEADDR;
    uint32_t lfsr = seed;
    uint32_t offsAddr = 0;

    //Fill all memory with pseudo-random data
    pc.printf(" - Filling all memory, starting with PR seed 0x%08x...   ", lfsr);
    for(offsAddr = 0; offsAddr < DRAMSIZE; offsAddr += 4) {
        lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xD0000001u);
        ramBaseAddr[offsAddr] = lfsr;
    }

    //force error at highest memory location
    //ramBaseAddr[0x007ffffc] = 0x12345678;

    //Now compare all that has been written
    pc.printf("done\r\n - Comparing all memory locations...   ");
    lfsr = seed;
    for(offsAddr = 0; offsAddr < DRAMSIZE; offsAddr += 4) {
        lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xD0000001u);
        if(ramBaseAddr[offsAddr] != lfsr) {
            pc.printf("\r\n### Memory compare error!!!  addr: %08x  sent: %08x  recvd: %08x\r\n", offsAddr + DRAMBASEADDR, lfsr, ramBaseAddr[offsAddr]);
            return -1;
        }
    }

    //print first DRAM locations
    pc.printf("done\r\n");

    //Option to print out first few locations of SDRAM
    //for(offsAddr = 0; offsAddr < 1024; offsAddr += 16) {
    //    pc.printf("addr: %08x  0: %08x  4: %08x  8: %08x  c: %08x\r\n", DRAMBASEADDR + offsAddr, ramBaseAddr[offsAddr],
    //                ramBaseAddr[offsAddr + 4], ramBaseAddr[offsAddr + 8], ramBaseAddr[offsAddr + 12]);
    //}
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------//
//Initialize the 32MB SDRAM and test it.

#define ONEMBDRAMSIZE (1024*1024)

int alloccount = 0;
unsigned char *ptr[100] ;

int initSDRAM() {
    pc.printf("Initializing 32MB SDRAM\r\n");
    if (sdram_init() == 1) {
        pc.printf(" - Failed to initialize SDRAM\r\n");
        return 1;
    }

    while(1) {
        ptr[alloccount] = (unsigned char *)malloc(ONEMBDRAMSIZE);
        //pc.printf (" - Allocated Address = %08x   %2d\r\n", ptr[alloccount], alloccount);
        if(ptr[alloccount] == NULL) {
            pc.printf(" - Maximum allocatable DRAM memory %dMB\r\n", alloccount);
            for (int i = 0; i < alloccount; i++) {
                free(ptr[i]);
            }
            alloccount = 0;
            break;
        } else {
            alloccount++ ;
        }
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------------------------//

int main(void) {

    //pc.baud(460800);
    pc.baud(115200);
    pc.printf("\r\n\r\nLPC4088 SDRAM memory test\r\n");
    initSDRAM();

    uint32_t StartSeed = 0xbad1deaf;
    int testresult = testSDRAM(StartSeed);

    while(1) {
        wait_ms(500);
        led1 = !led1;
    }
}
*/
