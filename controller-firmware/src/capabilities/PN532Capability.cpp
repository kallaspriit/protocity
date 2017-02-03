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

	nfc->checkForTag();
}

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

	printf("# enabling PN532 NFC tag reader\n");

	spi = new SPI(mosiPin, misoPin, sclkPin);
	nfc = new NFC(spi, portController->getPinName());

	nfc->addEventListener(this);

	if (!nfc->initialize()) {
		return false;
	}

	isEnabled = true;

	return true;
}

void PN532Capability::disable() {
	if (!isEnabled || nfc == NULL) {
		return;
	}

	printf("# disabling PN532 NFC tag reader\n");

	delete nfc;
	nfc = NULL;

	delete spi;
	spi = NULL;

	isEnabled = false;
}

void PN532Capability::onTagRead(NfcTag &tag) {
	// printf("# read '%s' tag with uid: %s\n", tag.getTagType().c_str(), tag.getUidString().c_str());
}

void PN532Capability::onTagEnter(NfcTag &tag) {
	// printf("# enter '%s' tag with uid: %s\n", tag.getTagType().c_str(), tag.getUidString().c_str());

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
		std::string payload = nfc->getRecordPayload(record);

		if (recordType == "T" && payload.size() > 0) {
			tagName = payload;

			break;
		}
	}

	if (tagName.size() > 0) {
		// printf("# '%s' (%s) ENTER\n", tagName.c_str(), tag.getUidString().c_str());

		snprintf(sendBuffer, SEND_BUFFER_SIZE, "enter:%s", tagName.c_str());

		portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));

		activeTagName = tagName;
	}
}

void PN532Capability::onTagExit(std::string lastTagUid) {
	if (activeTagName.size() == 0) {
		return;
	}

	// printf("# '%s' (%s) EXIT\n", activeTagName.c_str(), lastTagUid.c_str());

	snprintf(sendBuffer, SEND_BUFFER_SIZE, "exit:%s", activeTagName.c_str());

	portController->emitCapabilityUpdate(getName(), std::string(sendBuffer));

	activeTagName = "";
}
