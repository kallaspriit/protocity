#include "MifareClassic.h"
#include "PN532_debug.h"

#include <string.h> // memset

#define BLOCK_SIZE 16
#define LONG_TLV_SIZE 4
#define SHORT_TLV_SIZE 2

#define MIFARE_CLASSIC ("Mifare Classic")

MifareClassic::MifareClassic(PN532& nfcShield)
{
  _nfcShield = &nfcShield;
}

MifareClassic::~MifareClassic()
{
}

NfcTag MifareClassic::read(uint8_t *uid, unsigned int uidLength)
{
    uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
    int currentBlock = 4;
    int messageStartIndex = 0;
    int messageLength = 0;
    uint8_t data[BLOCK_SIZE];

    // read first block to get message length
    int success = _nfcShield->mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, key);
    if (success)
    {
        success = _nfcShield->mifareclassic_ReadDataBlock(currentBlock, data);
        if (success)
        {
            if (!decodeTlv(data, messageLength, messageStartIndex)) {
                return NfcTag(uid, uidLength, "ERROR"); // TODO should the error message go in NfcTag?
            }
        }
        else
        {
            DMSG("Reading block %d failed\n", currentBlock);

            return NfcTag(uid, uidLength, MIFARE_CLASSIC);
        }
    }
    else
    {
        DMSG("Tag is not NDEF formatted.\n");
        // TODO set tag.isFormatted = false
        return NfcTag(uid, uidLength, MIFARE_CLASSIC);
    }

    // this should be nested in the message length loop
    int index = 0;
    int bufferSize = getBufferSize(messageLength);
    uint8_t buffer[bufferSize];

    #ifdef MIFARE_CLASSIC_DEBUG
    DMSG("Message length: %d, buffer size: %d\n", messageLength, bufferSize);
    #endif

	bool didReadFail = false;

    while (index < bufferSize)
    {

        // authenticate on every sector
        if (_nfcShield->mifareclassic_IsFirstBlock(currentBlock))
        {
            success = _nfcShield->mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, key);
            if (!success)
            {
                DMSG("Block Authentication failed for block %d\n", currentBlock);
                // TODO error handling

				didReadFail = true;
            }
        }

        // read the data
        success = _nfcShield->mifareclassic_ReadDataBlock(currentBlock, &buffer[index]);

        if (success)
        {
            #ifdef MIFARE_CLASSIC_DEBUG
            DMSG("Read block %d\n", currentBlock);
            _nfcShield->PrintHexChar(&buffer[index], BLOCK_SIZE);
            #endif
        }
        else
        {
            DMSG("Reading mifare classic block %d failed\n", currentBlock);
            // TODO handle errors here

			didReadFail = true;
        }

        index += BLOCK_SIZE;
        currentBlock++;

        // skip the trailer block
        if (_nfcShield->mifareclassic_IsTrailerBlock(currentBlock))
        {
            #ifdef MIFARE_CLASSIC_DEBUG
            DMSG("Skipping block %d\n", currentBlock);
            #endif
            currentBlock++;
        }
    }

	if (didReadFail) {
		DMSG("Reading failed, returning empty tag\n");

		return NfcTag(uid, uidLength, MIFARE_CLASSIC);
	}

    return NfcTag(uid, uidLength, MIFARE_CLASSIC, &buffer[messageStartIndex], messageLength);
}

int MifareClassic::getBufferSize(int messageLength)
{

    int bufferSize = messageLength;

    // TLV header is 2 or 4 bytes, TLV terminator is 1 byte.
    if (messageLength < 0xFF)
    {
        bufferSize += SHORT_TLV_SIZE + 1;
    }
    else
    {
        bufferSize += LONG_TLV_SIZE + 1;
    }

    // bufferSize needs to be a multiple of BLOCK_SIZE
    if (bufferSize % BLOCK_SIZE != 0)
    {
        bufferSize = ((bufferSize / BLOCK_SIZE) + 1) * BLOCK_SIZE;
    }

    return bufferSize;
}

// skip null tlvs (0x0) before the real message
// technically unlimited null tlvs, but we assume
// T & L of TLV in the first block we read
int MifareClassic::getNdefStartIndex(uint8_t *data)
{

    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        if (data[i] == 0x0)
        {
            // do nothing, skip
        }
        else if (data[i] == 0x3)
        {
            return i;
        }
        else
        {
            DMSG("Unknown TLV: 0x%X\n", data[i]);

            return -2;
        }
    }

    return -1;
}

// Decode the NDEF data length from the Mifare TLV
// Leading null TLVs (0x0) are skipped
// Assuming T & L of TLV will be in the first block
// messageLength and messageStartIndex written to the parameters
// success or failure status is returned
//
// { 0x3, LENGTH }
// { 0x3, 0xFF, LENGTH, LENGTH }
bool MifareClassic::decodeTlv(uint8_t *data, int &messageLength, int &messageStartIndex)
{
    int i = getNdefStartIndex(data);

    if (i < 0 || data[i] != 0x3)
    {
        DMSG("Error. Can't decode message length.\n");
        return false;
    }
    else
    {
        if (data[i+1] == 0xFF)
        {
            messageLength = ((0xFF & data[i+2]) << 8) | (0xFF & data[i+3]);
            messageStartIndex = i + LONG_TLV_SIZE;
        }
        else
        {
            messageLength = data[i+1];
            messageStartIndex = i + SHORT_TLV_SIZE;
        }
    }

    return true;
}

// Intialized NDEF tag contains one empty NDEF TLV 03 00 FE - AN1304 6.3.1
// We are formatting in read/write mode with a NDEF TLV 03 03 and an empty NDEF record D0 00 00 FE - AN1304 6.3.2
bool MifareClassic::formatNDEF(uint8_t * uid, unsigned int uidLength)
{
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t emptyNdefMesg[16] = {0x03, 0x03, 0xD0, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer0[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    bool success = _nfcShield->mifareclassic_AuthenticateBlock (uid, uidLength, 0, 0, keya);
    if (!success)
    {
        DMSG("Unable to authenticate block 0 to enable card formatting!\n");
        return false;
    }
    success = _nfcShield->mifareclassic_FormatNDEF();
    if (!success)
    {
        DMSG("Unable to format the card for NDEF\n");
    }
    else
    {
        for (int i=4; i<64; i+=4) {
            success = _nfcShield->mifareclassic_AuthenticateBlock (uid, uidLength, i, 0, keya);

            if (success) {
                if (i == 4)  // special handling for block 4
                {
                    if (!(_nfcShield->mifareclassic_WriteDataBlock (i, emptyNdefMesg)))
                    {
                        DMSG("Unable to write block %d\n", i);
                    }
                }
                else
                {
                    if (!(_nfcShield->mifareclassic_WriteDataBlock (i, sectorbuffer0)))
                    {
                        DMSG("Unable to write block %d\n", i);
                    }
                }
                if (!(_nfcShield->mifareclassic_WriteDataBlock (i+1, sectorbuffer0)))
                {
                    DMSG("Unable to write block %d\n", i+1);
                }
                if (!(_nfcShield->mifareclassic_WriteDataBlock (i+2, sectorbuffer0)))
                {
                    DMSG("Unable to write block %d\n", i+2);
                }
                if (!(_nfcShield->mifareclassic_WriteDataBlock (i+3, sectorbuffer4)))
                {
                    DMSG("Unable to write block %d\n", i+3);
                }
            } else {
                unsigned int iii=uidLength;
                DMSG("Unable to authenticate block %d\n", i);
                _nfcShield->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, (uint8_t*)&iii);
            }
        }
    }
    return success;
}

#define NR_SHORTSECTOR          (32)    // Number of short sectors on Mifare 1K/4K
#define NR_LONGSECTOR           (8)     // Number of long sectors on Mifare 4K
#define NR_BLOCK_OF_SHORTSECTOR (4)     // Number of blocks in a short sector
#define NR_BLOCK_OF_LONGSECTOR  (16)    // Number of blocks in a long sector

// Determine the sector trailer block based on sector number
#define BLOCK_NUMBER_OF_SECTOR_TRAILER(sector) (((sector)<NR_SHORTSECTOR)? \
  ((sector)*NR_BLOCK_OF_SHORTSECTOR + NR_BLOCK_OF_SHORTSECTOR-1):\
  (NR_SHORTSECTOR*NR_BLOCK_OF_SHORTSECTOR + (sector-NR_SHORTSECTOR)*NR_BLOCK_OF_LONGSECTOR + NR_BLOCK_OF_LONGSECTOR-1))

bool MifareClassic::formatMifare(uint8_t * uid, unsigned int uidLength)
{

    // The default Mifare Classic key
    uint8_t KEY_DEFAULT_KEYAB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    uint8_t blockBuffer[16];                          // Buffer to store block contents
    uint8_t blankAccessBits[3] = { 0xff, 0x07, 0x80 };
    uint8_t idx = 0;
    uint8_t numOfSector = 16;                         // Assume Mifare Classic 1K for now (16 4-block sectors)
    bool success = false;

    for (idx = 0; idx < numOfSector; idx++)
    {
        // Step 1: Authenticate the current sector using key B 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
        success = _nfcShield->mifareclassic_AuthenticateBlock (uid, uidLength, BLOCK_NUMBER_OF_SECTOR_TRAILER(idx), 1, (uint8_t*)KEY_DEFAULT_KEYAB);
        if (!success)
        {
            DMSG("Authentication failed for sector %d\n", idx);
            return false;
        }

        // Step 2: Write to the other blocks
        if (idx == 0)
        {
            memset(blockBuffer, 0, sizeof(blockBuffer));
            if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer)))
            {
                DMSG("Unable to write to sector %d\n", idx);
            }
        }
        else
        {
            memset(blockBuffer, 0, sizeof(blockBuffer));
            // this block has not to be overwritten for block 0. It contains Tag id and other unique data.
            if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, blockBuffer)))
            {
                DMSG("Unable to write to sector %d\n", idx);
            }
            if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer)))
            {
                DMSG("Unable to write to sector %d\n", idx);
            }
        }

        memset(blockBuffer, 0, sizeof(blockBuffer));

        if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 1, blockBuffer)))
        {
            DMSG("Unable to write to sector %d\n", idx);
        }

        // Step 3: Reset both keys to 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
        memcpy(blockBuffer, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));
        memcpy(blockBuffer + 6, blankAccessBits, sizeof(blankAccessBits));
        blockBuffer[9] = 0x69;
        memcpy(blockBuffer + 10, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));

        // Step 4: Write the trailer block
        if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)), blockBuffer)))
        {
            DMSG("Unable to write trailer block of sector %d\n", idx);
        }
    }
    return true;
}

bool MifareClassic::write(NdefMessage& m, uint8_t * uid, unsigned int uidLength)
{

    uint8_t encoded[m.getEncodedSize()];
    m.encode(encoded);

    uint8_t buffer[getBufferSize(sizeof(encoded))];
    memset(buffer, 0, sizeof(buffer));

    #ifdef MIFARE_CLASSIC_DEBUG
    DMSG("sizeof(encoded):%d, sizeof(buffer): %d\n", sizeof(encoded), sizeof(buffer));
    #endif

    if (sizeof(encoded) < 0xFF)
    {
        buffer[0] = 0x3;
        buffer[1] = sizeof(encoded);
        memcpy(&buffer[2], encoded, sizeof(encoded));
        buffer[2+sizeof(encoded)] = 0xFE; // terminator
    }
    else
    {
        buffer[0] = 0x3;
        buffer[1] = 0xFF;
        buffer[2] = ((sizeof(encoded) >> 8) & 0xFF);
        buffer[3] = (sizeof(encoded) & 0xFF);
        memcpy(&buffer[4], encoded, sizeof(encoded));
        buffer[4+sizeof(encoded)] = 0xFE; // terminator
    }

    // Write to tag
    int index = 0;
    int currentBlock = 4;
    uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 }; // this is Sector 1 - 15 key

    while (index < sizeof(buffer))
    {

        if (_nfcShield->mifareclassic_IsFirstBlock(currentBlock))
        {
            int success = _nfcShield->mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, key);
            if (!success)
            {
                DMSG("Error. Block Authentication failed for %d\n", currentBlock);
                return false;
            }
        }

        int write_success = _nfcShield->mifareclassic_WriteDataBlock (currentBlock, &buffer[index]);
        if (write_success)
        {
            #ifdef MIFARE_CLASSIC_DEBUG
            DMSG("Wrote block %d - ", currentBlock);
            _nfcShield->PrintHexChar(&buffer[index], BLOCK_SIZE);
            #endif
        }
        else
        {
            DMSG("Writing block %d failed\n", currentBlock);
            return false;
        }
        index += BLOCK_SIZE;
        currentBlock++;

        if (_nfcShield->mifareclassic_IsTrailerBlock(currentBlock))
        {
            // can't write to trailer block
            #ifdef MIFARE_CLASSIC_DEBUG
            DMSG("Skipping block %d\n", currentBlock);
            #endif
            currentBlock++;
        }

    }

    return true;
}
