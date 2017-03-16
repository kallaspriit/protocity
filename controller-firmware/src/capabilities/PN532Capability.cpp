#include "PN532Capability.hpp"

#include "../PortController.hpp"

PN532Capability::PN532Capability(Serial *serial, PortController *portController, PinName mosiPin, PinName misoPin, PinName sclkPin) :
	AbstractCapability(serial, portController),
	mosiPin(mosiPin),
	misoPin(misoPin),
	sclkPin(sclkPin)
{}

std::string PN532Capability::getName() {
	return "PN532";
}

void PN532Capability::update(int deltaUs) {
	if (!isEnabled) {
		return;
	}

	nfc->update();
}

/*
void PN532Capability::runUpdateThread() {
	while (isEnabled) {
		//nfc->update();
		nfc->checkForTag();
	}
}
*/

CommandManager::Command::Response PN532Capability::handleCommand(CommandManager::Command *command) {
	if (command->argumentCount < 3) {
        return command->createFailureResponse("no capability action requested");
    }

	std::string action = command->getString(2);

	if (action == "enable") {
		return handleEnableCommand(command);
	} else if (action == "disable") {
		return handleDisableCommand(command);
	} else {
		return command->createFailureResponse("invalid capability action requested");
	}
}

CommandManager::Command::Response PN532Capability::handleEnableCommand(CommandManager::Command *command) {
	if (enable()) {
		return command->createSuccessResponse();
	} else {
		return command->createFailureResponse("enabling nfc tag reader failed");
	}
}

CommandManager::Command::Response PN532Capability::handleDisableCommand(CommandManager::Command *command) {
	disable();

	return command->createSuccessResponse();
}

bool PN532Capability::enable() {
	if (isEnabled) {
		return true;
	}

	log.info("enabling PN532 NFC tag reader");

	spi = new SPI(mosiPin, misoPin, sclkPin);
	nfc = new NFC(spi, portController->getPinName());

	nfc->addEventListener(this);

	if (!nfc->initialize()) {
		return false;
	}

	isEnabled = true;

	//updateThread.start(callback(this, &PN532Capability::runUpdateThread));

	return true;
}

void PN532Capability::disable() {
	if (!isEnabled || nfc == NULL) {
		return;
	}

	log.info("disabling PN532 NFC tag reader");

	delete nfc;
	nfc = NULL;

	delete spi;
	spi = NULL;

	isEnabled = false;
}

void PN532Capability::onTagRead(NfcTag &tag) {
	log.trace("read '%s' tag with uid: %s", tag.getTagType().c_str(), tag.getUidString().c_str());
}

void PN532Capability::onTagEnter(NfcTag &tag) {
	// report the uid even if there is no payload
	snprintf(sendBuffer, SEND_BUFFER_SIZE, "uid:enter:%s", tag.getUidString().c_str());
	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));

	if (!tag.hasNdefMessage()) {
		log.trace("enter %s but it does not contain a ndef message (%s)", tag.getUidString().c_str(), tag.getTagType().c_str());

		return;
	}

	NdefMessage message = tag.getNdefMessage();
	int recordCount = message.getRecordCount();

	if (recordCount == 0) {
		log.trace("enter %s but it contains no records (%s)", tag.getUidString().c_str(), tag.getTagType().c_str());

		return;
	}

	std::string tagName = "";

	for (int i = 0; i < recordCount; i++) {
		NdefRecord record = message.getRecord(i);
		std::string recordId = record.getId();
		std::string recordType = record.getType();
		std::string payload = nfc->getRecordPayload(record);

		if (recordType == "T" && payload.size() > 0) {
			tagName = payload;

			break;
		}
	}

	if (tagName.size() == 0) {
		log.trace("no text record with tag name could be found (uid: %s)", tag.getUidString().c_str());

		return;
	}

	if (tagName != activeTagName) {
		log.debug("ENTER '%s' (uid: %s)", tagName.c_str(), tag.getUidString().c_str());

		snprintf(sendBuffer, SEND_BUFFER_SIZE, "enter:%s", tagName.c_str());
		portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));

		activeTagName = tagName;
	}
}

void PN532Capability::onTagExit(std::string lastTagUid) {
	// report the uid even if there is no payload
	snprintf(sendBuffer, SEND_BUFFER_SIZE, "uid:exit:%s", lastTagUid.c_str());
	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));

	if (activeTagName.size() == 0) {
		return;
	}

	log.debug("EXIT '%s' (uid: %s)", activeTagName.c_str(), lastTagUid.c_str());

	snprintf(sendBuffer, SEND_BUFFER_SIZE, "exit:%s", activeTagName.c_str());

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));

	activeTagName = "";
}
