#include "mbed.h"
#include "rtos.h"

#include <sdram.h>

#include <PN532_SPI.h>
#include <PN532.h>
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
//PN532 nfc(interface);
//SNEP nfc(interface);
NfcAdapter nfc = NfcAdapter(interface);
DigitalOut led(LED1);
int counter = 0;

void setup() {
	pc.printf("# setting up nfc module.. ");


	if (!nfc.begin()) {
		error("module not found!");
	}

	pc.printf("done!\n");

	uint32_t versiondata = nfc.getVersionInfo();

	printf("# found chip PN50x%X, firmware version: %d.%d\n", versiondata>>24, (versiondata>>16) & 0xFF, (versiondata>>8) & 0xFF);

	/*
	uint32_t versiondata = nfc.getFirmwareVersion();

	if (!versiondata) {
		error("Didn't find PN53x board\n");
	}

	// Got ok data, print it out!
	printf("Found chip PN5 0x%X, firmware: %d.%d\n", versiondata>>24, (versiondata>>16) & 0xFF, (versiondata>>8) & 0xFF);

	// configure board to read RFID tags
	nfc.SAMConfig();
	*/
}

void loop() {
	led = !led;

	// pc.printf("\n# checking for tag\n");

	std::string tagName = "";

	if (nfc.tagPresent()) {
		NfcTag tag = nfc.read();

        //tag.print();

		if (tag.hasNdefMessage()) {
            NdefMessage message = tag.getNdefMessage();
			int recordCount = message.getRecordCount();

			pc.printf("# found a '%s' tag containing a message with %d records:\n", tag.getTagType().c_str(), recordCount);

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
				} else {
					pc.printf("# - %d ignoring unsupported record type: '%s'\n", i, recordType.c_str());

					continue;
				}

				for (int j = payloadStartIndex; j < payloadLength; j++) {
					payloadAsString += (char)payload[j];
				}

				tagName = payloadAsString;

				pc.printf("# - %d type: %s '%s'\n", i, recordType.c_str(), payloadAsString.c_str());
			}
        } else {
			// pc.printf("found tag but it does not contain a message\n");
		}

		//Thread::wait(500);

    } else {
		// pc.printf("# no tag was detected\n");
    }

	/*
	if (tagName.size() > 0) {
		pc.printf("\n#%.4d detected tag: '%s'\n", ++counter, tagName.c_str());
	} else {
		pc.printf(".");
	}
	*/
}
/*
void loop() {
	uint8_t success;
	uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
	uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

	// Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
	// 'uid' will be populated with the UID, and uidLength will indicate
	// if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
	success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

	if (success) {
		printf("found tag with uid 0x%X (length: %d)\n", uid, uidLength);

		if (uidLength == 4) {
			// We probably have a Mifare Classic card ...
			printf("Seems to be a Mifare Classic card (4 uint8_t UID), trying to authenticate block 4 with default KEYA value\n");

			// Now we need to try to authenticate it for read/write access
			// Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
			uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

			// Start with block 4 (the first block of sector 1) since sector 0
			// contains the manufacturer data and it's probably better just
			// to leave it alone unless you know what you're doing
			success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);

			if (success) {
				printf("Sector 1 (Blocks 4..7) has been authenticated\n");
				uint8_t data[16];

				// If you want to write something to block 4 to test with, uncomment
				// the following line and this text should be read back in a minute
				// data = { 'a', 'd', 'a', 'f', 'r', 'u', 'i', 't', '.', 'c', 'o', 'm', 0, 0, 0, 0};
				// success = nfc.mifareclassic_WriteDataBlock (4, data);

				// Try to read the contents of block 4
				success = nfc.mifareclassic_ReadDataBlock(4, data);

				if (success) {
					// Data seems to have been read ... spit it out
					printf("Reading Block 4:");
					nfc.PrintHexChar(data, 16);
					printf("\n");

					// Wait a bit before reading the card again
					Thread::wait(1000);
				} else {
					printf("Ooops ... unable to read the requested block.  Try another key?\n");
				}
			} else {
				printf("Ooops ... authentication failed: Try another key?\n");
			}
		} else if (uidLength == 7) {
			// We probably have a Mifare Ultralight card ...
			printf("Seems to be a Mifare Ultralight tag (7 uint8_t UID)\n");

			*
			uint8_t ndefBuf[128];

			int msgSize = nfc2.read(ndefBuf, sizeof(ndefBuf));

			if (msgSize > 0) {
				NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
				msg.print();
				printf("success\n");
			} else {
				printf("failed\n");
			}
			*

			*
			// Try to read the first general-purpose user page (#4)
			printf("Reading page 4\n");
			uint8_t data[32];
			success = nfc.mifareultralight_ReadPage (4, data);

			if (success) {
				printf("read successful:\n");

				// Data seems to have been read ... spit it out
				nfc.PrintHexChar(data, 4);
				printf("\n");

				// Wait a bit before reading the card again
				Thread::wait(1000);
			} else {
				printf("Ooops ... unable to read the requested page!?\n");
			}
			*
		} else {
			printf("Unknown tag with uid length of %d was detected\n", uidLength);
		}
	} else {
		printf("no tag was found\n");
	}
}
*/
/*
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
				string payloadAsstring = "";
				int payloadStartIndex = 0;

				if (recordType == RECORD_TYPE_TEXT) {
					// first 3 bits are encofing, RFU and length on language code
					payloadStartIndex = 3;
				} else if (recordType == RECORD_TYPE_URL) {
					payloadStartIndex = 1;
				}

				for (int j = payloadStartIndex; j < payloadLength; j++) {
					payloadAsstring += (char)payload[j];
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
*/

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
