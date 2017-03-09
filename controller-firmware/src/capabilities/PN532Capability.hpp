#ifndef PN532CAPABILITY_HPP
#define PN532CAPABILITY_HPP

#include "../AbstractCapability.hpp"

#include "NFC.hpp"

#include <string>

class PN532Capability : public AbstractCapability, private NFC::NfcEventListener {

public:
	PN532Capability(Serial *serial, PortController *portController, PinName mosiPin, PinName misoPin, PinName sclkPin);

	std::string getName();
	void update(int deltaUs);

	CommandManager::Command::Response handleCommand(CommandManager::Command *command);

private:
	Log log = Log::getLog("PN532Capability");

	CommandManager::Command::Response handleEnableCommand(CommandManager::Command *command);
	CommandManager::Command::Response handleDisableCommand(CommandManager::Command *command);

	bool enable();
	void disable();

	//void runUpdateThread();

	void onTagRead(NfcTag &tag) override;
	void onTagEnter(NfcTag &tag) override;
	void onTagExit(std::string lastTagUid) override;

	//Thread updateThread;

	const PinName mosiPin;
	const PinName misoPin;
	const PinName sclkPin;

	SPI *spi = NULL;
	NFC *nfc = NULL;
	bool isEnabled = false;
	std::string activeTagName = "";
};

#endif
