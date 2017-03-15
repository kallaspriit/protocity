#include "NFC.hpp"

NFC::NFC(SPI &spi, PinName spiSlaveSelectPin) :
	spi(&spi),
	interface(spi, spiSlaveSelectPin),
	adapter(interface)
{}

NFC::NFC(SPI *spi, PinName spiSlaveSelectPin) :
	spi(spi),
	interface(spi, spiSlaveSelectPin),
	adapter(interface)
{}

bool NFC::initialize() {
	log.info("initializing");

	return adapter.begin();
}

NFC::Version NFC::getVersion() {
	uint32_t versiondata = adapter.getVersionInfo();

	return Version(
		(unsigned long)(versiondata>>24),
		(int)((versiondata>>16) & 0xFF),
		(int)((versiondata>>8) & 0xFF)
	);
}

void NFC::addEventListener(NfcEventListener *listener) {
	nfcEventListeners.push_back(listener);

	log.info("added event listener (%d total)", nfcEventListeners.size());
}

void NFC::checkForTag() {
	log.trace("checking for tag presence");

	// if (!adapter.tagPresent()) {
	if (!adapter.tagPresent(100)) {
		bool wasTagPresent = activeTagUid.size() > 0;

		if (wasTagPresent) {
			log.debug("tag %s exited", activeTagUid.c_str());

			for (NfcEventListenerList::iterator it = nfcEventListeners.begin(); it != nfcEventListeners.end(); ++it) {
				(*it)->onTagExit(activeTagUid);
			}
		}

		activeTagUid = "";

		return;
	}

	NfcTag tag = adapter.read();

	std::string uid = tag.getUidString();

	log.trace("tag %s was read", uid.c_str());

	// always send the read event
	for (NfcEventListenerList::iterator it = nfcEventListeners.begin(); it != nfcEventListeners.end(); ++it) {
		(*it)->onTagRead(tag);
	}

	// only change the enter event if the tag uid has changed (or there was no tag before)
	if (uid != activeTagUid) {
		log.debug("tag %s entered", uid.c_str());

		for (NfcEventListenerList::iterator it = nfcEventListeners.begin(); it != nfcEventListeners.end(); ++it) {
			(*it)->onTagEnter(tag);
		}
	}

	activeTagUid = uid;
}

void NFC::update() {
	if (!isCheckScheduled) {
		scheduleCheck();
	} else if (isCheckReady()) {
		performCheck();
		scheduleCheck();
	}
}

bool NFC::scheduleCheck() {
	if (!adapter.requestTagPresent()) {
		log.warn("requesting tag present failed");

		return false;
	}

	isCheckScheduled = true;
	checkScheduleTimer.reset();
	checkScheduleTimer.start();

	return true;
}

bool NFC::isCheckReady() {
	if (!isCheckScheduled) {
		return false;
	}

	//return adapter.isReady();

	int timeSinceCheckSchedule = checkScheduleTimer.read_ms();

	return timeSinceCheckSchedule >= CHECK_TIME_REQUIRED;
}

bool NFC::performCheck() {
	bool isTagPresent = adapter.checkTagPresent();

	isCheckScheduled = false;
	checkScheduleTimer.stop();

	if (isTagPresent) {
		NfcTag tag = adapter.read();

		std::string uid = tag.getUidString();

		log.trace("tag %s was read", uid.c_str());

		// always send the read event
		for (NfcEventListenerList::iterator it = nfcEventListeners.begin(); it != nfcEventListeners.end(); ++it) {
			(*it)->onTagRead(tag);
		}

		// only change the enter event if the tag uid has changed (or there was no tag before)
		if (uid != activeTagUid) {
			log.debug("tag %s entered", uid.c_str());

			for (NfcEventListenerList::iterator it = nfcEventListeners.begin(); it != nfcEventListeners.end(); ++it) {
				(*it)->onTagEnter(tag);
			}
		}

		activeTagUid = uid;

	} else {
		bool wasTagPresent = activeTagUid.size() > 0;

		if (wasTagPresent) {
			log.debug("tag %s exited", activeTagUid.c_str());

			for (NfcEventListenerList::iterator it = nfcEventListeners.begin(); it != nfcEventListeners.end(); ++it) {
				(*it)->onTagExit(activeTagUid);
			}
		}

		activeTagUid = "";
	}

	return isTagPresent;
}

std::string NFC::getRecordPayload(NdefRecord &record) {
	int payloadLength = record.getPayloadLength();

	if (payloadLength > MAX_PAYLOAD_SIZE) {
		return "";
	}

	record.getPayload(payload);
	string payloadAsString = "";
	int payloadStartIndex = 0;
	string recordType = record.getType();

	if (recordType == RECORD_TYPE_TEXT) {
		// first 3 bits are encofing, RFU and length on language code
		payloadStartIndex = 3;
	} else if (recordType == RECORD_TYPE_URL) {
		payloadStartIndex = 1;
	}

	for (int j = payloadStartIndex; j < payloadLength; j++) {
		payloadAsString += (char)payload[j];

		log.info("READ char %c (%d) to %s (%d total)", payload[j], payload[j], payloadAsString.c_str(), payloadLength);
	}

	return payloadAsString;
}
