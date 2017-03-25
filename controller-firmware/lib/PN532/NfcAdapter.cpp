#include <NfcAdapter.h>
#include <PN532_debug.h>

NfcAdapter::NfcAdapter(PN532Interface &interface)
{
    shield = new PN532(interface);
}

NfcAdapter::~NfcAdapter(void)
{
    delete shield;
}

bool NfcAdapter::begin()
{
    shield->begin();

    int attemptCount = 3;
    int attemptsLeft = attemptCount;
    uint32_t versiondata = 0;

    // does not always succeed on the first attempt for some reason..
    while (attemptsLeft > 0) {
        attemptsLeft--;

        versiondata = shield->getFirmwareVersion();

        if (versiondata != 0) {
            break;
        }
    }

    if (versiondata == 0)
    {
        DMSG("Didn't find PN53x board after %d attempts\n", attemptCount);

        return false;
    }

    DMSG("Found chip PN5%X after %d attempts, firmare: %d.%d\n", (versiondata>>24) & 0xFF, attemptCount - attemptsLeft, (versiondata>>16) & 0xFF, (versiondata>>8) & 0xFF);

    // configure board to read RFID tags (returns false for some reason?)
    shield->SAMConfig();

	return true;
}

uint32_t NfcAdapter::getVersionInfo() {
	return shield->getFirmwareVersion();
}

bool NfcAdapter::tagPresent(unsigned long timeout)
{
    uint8_t success;
    uidLength = 0;

    if (timeout == 0)
    {
        success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, (uint8_t*)&uidLength);
    }
    else
    {
        success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, (uint8_t*)&uidLength, timeout);
    }
    return success;
}

bool NfcAdapter::requestTagPresent() {
    return shield->requestTagPresent(PN532_MIFARE_ISO14443A);
}

bool NfcAdapter::checkTagPresent() {
    uidLength = 0;

    return shield->checkTagPresent(uid, (uint8_t*)&uidLength);
}

bool NfcAdapter::isReady() {
    return shield->isReady();
}

bool NfcAdapter::erase()
{
    bool success;
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return write(message);
}

bool NfcAdapter::format()
{
    bool success;
    if (uidLength == 4)
    {
        MifareClassic mifareClassic = MifareClassic(*shield);
        success = mifareClassic.formatNDEF(uid, uidLength);
    }
    else
    {
        DMSG("Unsupported Tag.");
        success = false;
    }
    return success;
}

bool NfcAdapter::clean()
{
    uint8_t type = guessTagType();

    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        #ifdef NDEF_DEBUG
        DMSG("Cleaning Mifare Classic");
        #endif
        MifareClassic mifareClassic = MifareClassic(*shield);
        return mifareClassic.formatMifare(uid, uidLength);
    }
    else if (type == TAG_TYPE_2)
    {
        #ifdef NDEF_DEBUG
        DMSG("Cleaning Mifare Ultralight");
        #endif
        MifareUltralight ultralight = MifareUltralight(*shield);
        return ultralight.clean();
    }
    else
    {
        DMSG("No driver for card type %d\n", type);
        return false;
    }

}


NfcTag NfcAdapter::read()
{
    uint8_t type = guessTagType();

    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        #ifdef NDEF_DEBUG
        DMSG("Reading Mifare Classic");
        #endif
        MifareClassic mifareClassic = MifareClassic(*shield);
        return mifareClassic.read(uid, uidLength);
    }
    else if (type == TAG_TYPE_2)
    {
        #ifdef NDEF_DEBUG
        DMSG("Reading Mifare Ultralight");
        #endif
        MifareUltralight ultralight = MifareUltralight(*shield);
        return ultralight.read(uid, uidLength);
    }
    else if (type == TAG_TYPE_UNKNOWN)
    {
        DMSG("Can not determine tag type");
        return NfcTag(uid, uidLength);
    }
    else
    {
        DMSG("No driver for card type %d\n", type);
        // TODO should set type here
        return NfcTag(uid, uidLength);
    }

}

bool NfcAdapter::write(NdefMessage& ndefMessage)
{
    bool success;
    uint8_t type = guessTagType();

    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        #ifdef NDEF_DEBUG
        DMSG("Writing Mifare Classic");
        #endif
        MifareClassic mifareClassic = MifareClassic(*shield);
        success = mifareClassic.write(ndefMessage, uid, uidLength);
    }
    else if (type == TAG_TYPE_2)
    {
        #ifdef NDEF_DEBUG
        DMSG("Writing Mifare Ultralight");
        #endif
        MifareUltralight mifareUltralight = MifareUltralight(*shield);
        success = mifareUltralight.write(ndefMessage, uid, uidLength);
    }
    else if (type == TAG_TYPE_UNKNOWN)
    {
        DMSG("Can not determine tag type");
        success = false;
    }
    else
    {
        DMSG("No driver for card type %d\n", type);
        success = false;
    }

    return success;
}

// TODO this should return a Driver MifareClassic, MifareUltralight, Type 4, Unknown
// Guess Tag Type by looking at the ATQA and SAK values
// Need to follow spec for Card Identification. Maybe AN1303, AN1305 and ???
unsigned int NfcAdapter::guessTagType()
{

    // 4 uint8_t id - Mifare Classic
    //  - ATQA 0x4 && SAK 0x8
    // 7 uint8_t id
    //  - ATQA 0x44 && SAK 0x8 - Mifare Classic
    //  - ATQA 0x44 && SAK 0x0 - Mifare Ultralight NFC Forum Type 2
    //  - ATQA 0x344 && SAK 0x20 - NFC Forum Type 4

    if (uidLength == 4)
    {
        return TAG_TYPE_MIFARE_CLASSIC;
    }
    else
    {
        return TAG_TYPE_2;
    }
}
