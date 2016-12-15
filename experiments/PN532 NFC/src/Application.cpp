#include "Application.hpp"

Application::Application(Serial *serial) :
 	serial(serial),
	spi(PN532_SPI_MOSI, PN532_SPI_MISO, PN532_SPI_SCK),
	nfc(spi, PN532_SPI_CHIP_SELECT),
	led(LED1)
{}

void Application::run() {
	setup();

	timer.start();

	while (true) {
		int deltaUs = timer.read_us();
		timer.reset();

		loop(deltaUs);
	}
}

void Application::setup() {
	serial->printf("# setting up nfc module.. ");

	nfc.addEventListener(this);

	if (!nfc.initialize()) {
		error("module not found!");
	}

	serial->printf("done!\n");

	NFC::Version version = nfc.getVersion();

	printf("# found chip PN50%X, firmware version: %d.%d\n", version.hardwareVersion, version.softwareMajorVersion, version.softwareMinorVersion);
}

void Application::loop(int deltaUs) {
	led = !led;

	// serial->printf("# checking for tag (%d us since last update)\n", deltaUs);

	nfc.checkForTag();

	//Thread::wait(1000);

	// constants
	//const char RECORD_TYPE_TEXT[] = "T";
	//const char RECORD_TYPE_URL[] = "U";

	/*
	std::string tagName = "";

	if (nfc.tagPresent()) {
		NfcTag tag = nfc.read();

        //tag.print();

		if (tag.hasNdefMessage()) {
            NdefMessage message = tag.getNdefMessage();
			int recordCount = message.getRecordCount();

			serial->printf("# found a '%s' tag containing a message with %d records:\n", tag.getTagType().c_str(), recordCount);

			for (int i = 0; i < recordCount; i++) {
				NdefRecord record = message.getRecord(i);
				string recordId = record.getId();
				string recordType = record.getType();

				int payloadLength = record.getPayloadLength();

				if (payloadLength > MAX_PAYLOAD_SIZE) {
					serial->printf("# payload is too large (%d > %d)", payloadLength, MAX_PAYLOAD_SIZE);

					continue;
				}

				record.getPayload(payload);
				string payloadAsString = "";
				int payloadStartIndex = 0;

				if (recordType == RECORD_TYPE_TEXT) {
					// first 3 bits are encofing, RFU and length on language code
					payloadStartIndex = 3;
				} else if (recordType == RECORD_TYPE_URL) {
					payloadStartIndex = 1;
				} else {
					serial->printf("# - %d ignoring unsupported record type: '%s'\n", i, recordType.c_str());

					continue;
				}

				for (int j = payloadStartIndex; j < payloadLength; j++) {
					payloadAsString += (char)payload[j];
				}

				tagName = payloadAsString;

				serial->printf("# - %d type: %s '%s'\n", i, recordType.c_str(), payloadAsString.c_str());
			}
        } else {
			// serial->printf("found tag but it does not contain a message\n");
		}

		//Thread::wait(500);

    } else {
		// serial->printf("# no tag was detected\n");
    }
	*/

	/*
	if (tagName.size() > 0) {
		serial->printf("\n#%.4d detected tag: '%s'\n", ++counter, tagName.c_str());
	} else {
		serial->printf(".");
	}
	*/
}

void Application::onTagRead(NfcTag &tag) {
	/*serial->printf("# read '%s' tag with uid: %s\n", tag.getTagType().c_str(), tag.getUidString().c_str());

	showTagInfo(tag);*/
}

void Application::onTagEnter(NfcTag &tag) {
	// serial->printf("# enter '%s' tag with uid: %s\n", tag.getTagType().c_str(), tag.getUidString().c_str());

	if (!tag.hasNdefMessage()) {
		return;
	}

	NdefMessage message = tag.getNdefMessage();
	int recordCount = message.getRecordCount();

	std::string tagName = "";

	for (int i = 0; i < recordCount; i++) {
		NdefRecord record = message.getRecord(i);
		std::string recordId = record.getId();
		std::string recordType = record.getType();
		std::string payload = nfc.getRecordPayload(record);

		if (recordType == "T" && payload.size() > 0) {
			tagName = payload;

			break;
		}
	}

	if (tagName.size() > 0) {
		serial->printf("< '%s' (%s) ENTER\n", tagName.c_str(), tag.getUidString().c_str());

		activeTagName = tagName;
	}
}

void Application::onTagExit(std::string lastTagUid) {
	if (activeTagName.size() == 0) {
		return;
	}

	serial->printf("> '%s' (%s) EXIT\n", activeTagName.c_str(), lastTagUid.c_str());

	activeTagName = "";
}

void Application::showTagInfo(NfcTag &tag) {
	if (tag.hasNdefMessage()) {
		NdefMessage message = tag.getNdefMessage();
		int recordCount = message.getRecordCount();

		serial->printf("# found a '%s' tag containing a message with %d records:\n", tag.getTagType().c_str(), recordCount);

		for (int i = 0; i < recordCount; i++) {
			NdefRecord record = message.getRecord(i);
			std::string recordId = record.getId();
			std::string recordType = record.getType();
			std::string payload = nfc.getRecordPayload(record);

			serial->printf("# - %d type: %s '%s'\n", i, recordType.c_str(), payload.c_str());
		}
	} else {
		serial->printf("# found tag but it does not contain a message\n");
	}
}
