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
}

void NFC::checkForTag() {
	unsigned long timeout = 30; // minimum that still works

	if (!adapter.tagPresent(timeout)) {
		bool wasTagPresent = activeTagUid.size() > 0;

		if (wasTagPresent) {
			for (NfcEventListenerList::iterator it = nfcEventListeners.begin(); it != nfcEventListeners.end(); ++it) {
				(*it)->onTagExit(activeTagUid);
			}
		}

		activeTagUid = "";

		return;
	}

	NfcTag tag = adapter.read();

	std::string uid = tag.getUidString();

	// always send the read event
	for (NfcEventListenerList::iterator it = nfcEventListeners.begin(); it != nfcEventListeners.end(); ++it) {
		(*it)->onTagRead(tag);
	}

	// only change the enter event if the tag uid has changed (or there was no tag before)
	if (uid != activeTagUid) {
		for (NfcEventListenerList::iterator it = nfcEventListeners.begin(); it != nfcEventListeners.end(); ++it) {
			(*it)->onTagEnter(tag);
		}
	}

	activeTagUid = uid;
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
	}

	return payloadAsString;
}
