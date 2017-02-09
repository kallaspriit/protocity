#ifndef NFC_HPP
#define NFC_HPP

#include "mbed.h"

#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

#include <vector>
#include <string>

class NFC {

public:
	struct Version {

		Version(unsigned long hardwareVersion, int softwareMajorVersion, int softwareMinorVersion) :
			hardwareVersion(hardwareVersion),
			softwareMajorVersion(softwareMajorVersion),
			softwareMinorVersion(softwareMinorVersion)
		{}

		unsigned long hardwareVersion;
		int softwareMajorVersion;
		int softwareMinorVersion;

	};

	class NfcEventListener {

	public:
		virtual void onTagRead(NfcTag &tag) {}
		virtual void onTagEnter(NfcTag &tag) {}
		virtual void onTagExit(std::string lastTagUid) {}

	};

	NFC(SPI &spi, PinName spiSlaveSelectPin);
	NFC(SPI *spi, PinName spiSlaveSelectPin);

	bool initialize();
	Version getVersion();
	void addEventListener(NfcEventListener *listener);

	void checkForTag();

	void update();
	bool scheduleCheck();
	bool performCheck();
	bool isCheckReady();

	std::string getRecordPayload(NdefRecord &record);

	bool isCheckScheduled = false;

private:
	// dependencies
	SPI *spi;
	PN532_SPI interface;
	NfcAdapter adapter;

	// configuration
	static const int MAX_PAYLOAD_SIZE = 128;
	// static const int CHECK_TIME_REQUIRED = 50; // ms
	static const int CHECK_TIME_REQUIRED = 1000; // ms

	// listeners
	typedef std::vector<NfcEventListener*> NfcEventListenerList;
	NfcEventListenerList nfcEventListeners;

	// constants
	const char *RECORD_TYPE_TEXT = "T";
	const char *RECORD_TYPE_URL = "U";

	// runtime
	uint8_t payload[MAX_PAYLOAD_SIZE];
	std::string activeTagUid = "";

	// update
	Timer checkScheduleTimer;

};

#endif
