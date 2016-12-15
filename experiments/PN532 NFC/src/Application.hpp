#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "mbed.h"
#include "rtos.h"

#include "NFC.hpp"

class Application : public NFC::NfcEventListener {

public:
	Application(Serial *serial);

	void run();
	void setup();
	void loop(int deltaUs);

	void onTagRead(NfcTag &tag) override;
	void onTagEnter(NfcTag &tag) override;
	void onTagExit(std::string lastTagUid) override;

private:
	void showTagInfo(NfcTag &tag);

	// configuration
	static const PinName PN532_SPI_MOSI = p5;
	static const PinName PN532_SPI_MISO = p6;
	static const PinName PN532_SPI_SCK = p7;
	static const PinName PN532_SPI_CHIP_SELECT = p26;
	static const PinName SERIAL_TX = USBTX;
	static const PinName SERIAL_RX = USBRX;

	Serial *serial;
	SPI spi;
	NFC nfc;
	DigitalOut led;
	Timer timer;

	std::string activeTagName = "";

};

#endif
