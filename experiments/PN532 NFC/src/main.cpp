#include "mbed.h"
#include "rtos.h"

#include <sdram.h>

#include <PN532_SPI.h>
#include <NfcAdapter.h>

// based on https://developer.mbed.org/users/yihui/code/PN532/

// configuration
const PinName PN532_SPI_MOSI = p5;
const PinName PN532_SPI_MISO = p6;
const PinName PN532_SPI_SCK = p7;
const PinName PN532_SPI_CHIP_SELECT = p26;
const PinName SERIAL_TX = USBTX;
const PinName SERIAL_RX = USBRX;

// constants
const char RECORD_TYPE_TEXT[] = "T";
const char RECORD_TYPE_URL[] = "U";

// instances
Serial pc(SERIAL_TX, SERIAL_RX);
SPI spi(PN532_SPI_MOSI, PN532_SPI_MISO, PN532_SPI_SCK);
PN532_SPI interface(spi, PN532_SPI_CHIP_SELECT);
NfcAdapter nfc(interface);
DigitalOut led(LED1);

void setup() {
	pc.printf("# setting up nfc module.. ");

	nfc.begin();

	pc.printf("done!\n");
}

void loop() {
	pc.printf("# detecting tag..");

	led = !led;

	if (nfc.tagPresent()) {
		NfcTag tag = nfc.read();        // tag.print() doesn't work yet

        if (tag.hasNdefMessage()) {
            NdefMessage message = tag.getNdefMessage();
			int recordCount = message.getRecordCount();

			pc.printf("found tag containing a message with %d records:\n", recordCount);

			for (int i = 0; i < recordCount; i++) {
				NdefRecord record = message.getRecord(i);
				int idLength = record.getIdLength();
				string recordId = record.getId();
				string recordType = record.getType();
				int tnf = record.getTnf();

				int payloadLength = record.getPayloadLength();
				uint8_t payload[payloadLength];
				record.getPayload(payload);
				string payloadAsString = "";
				int payloadStartIndex = 0;

				if (recordType == RECORD_TYPE_TEXT) {
					// first 3 bits are encofing, RFU and length on language code
					payloadStartIndex = 3;
				} else if (recordType == RECORD_TYPE_URL) {
					payloadStartIndex = 1;
				}

				for (int j = payloadStartIndex; j < payloadLength; j++) {
					payloadAsString += (char)payload[j];
				}

				pc.printf("#   - %d: id: %s(%d), tnf: 0x%.2X, type: %s, content: %s(%d)\n", i, recordId.c_str(), idLength, tnf, recordType.c_str(), payloadAsString.c_str(), payloadLength);
			}
        } else {
			pc.printf("found tag but it does not contain a message\n");
		}

		Thread::wait(500);
    } else {
		pc.printf("no tag was detected\n");
	}

    //wait(5.0f);
}

int main() {
	pc.baud(115200);

	if (sdram_init() != 0) {
        error("failed to initialize SDRAM\n");
    }

	pc.printf("\n\n### Starting PN532 NFC module test ###\n");

	setup();

	while (true) {
		loop();
	}
}
