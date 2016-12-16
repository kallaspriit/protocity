#ifndef PN532CAPABILITY_HPP
#define PN532CAPABILITY_HPP

#include "../AbstractCapability.hpp"

#include "NFC.hpp"

#include <string>

class PN532Capability : public AbstractCapability, public NFC::NfcEventListener {

public:
	PN532Capability(Serial *serial, PortController *portController, PinName slaveSelectPin);

	std::string getName();
	CommandManager::Command::Response execute(CommandManager::Command *command);
	void update(int deltaUs);

	void onTagRead(NfcTag &tag) override;
	void onTagEnter(NfcTag &tag) override;
	void onTagExit(std::string lastTagUid) override;

private:
	bool enable();
	void disable();

	PinName slaveSelectPin;
	SPI *spi = NULL;
	NFC *nfc = NULL;
	bool isEnabled = false;
	std::string activeTagName = "";

	static const int SEND_BUFFER_SIZE = 64;
	char sendBuffer[SEND_BUFFER_SIZE];
};

#endif
