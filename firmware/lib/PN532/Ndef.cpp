#include "Ndef.h"
#include "PN532_debug.h"

// Borrowed from Adafruit_NFCShield_I2C
void PrintHex(const uint8_t * data, const long numBytes)
{
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++)
  {
    DMSG("0x");
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      DMSG("0");
    DMSG("%0x%X", data[szPos]&0xff);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      DMSG(" ");
    }
  }
  DMSG("\n");
}

// Borrowed from Adafruit_NFCShield_I2C
void PrintHexChar(const uint8_t * data, const long numBytes)
{
  uint32_t szPos;

  for (szPos=0; szPos < numBytes; szPos++)
  {
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      DMSG("0x%X", data[szPos]);

      if ((numBytes > 1) && (szPos != numBytes - 1))
      {
        DMSG(" ");
      }
  }

  DMSG(" ");

  for (szPos=0; szPos < numBytes; szPos++)
  {
    if (data[szPos] <= 0x1F)
      DMSG(".");
    else
      DMSG("%c", (char)data[szPos]);
  }

  DMSG("\n");
}

// Note if buffer % blockSize != 0, last block will not be written
void DumpHex(const uint8_t * data, const long numBytes, const unsigned int blockSize)
{
    int i;
    for (i = 0; i < (numBytes / blockSize); i++)
    {
        PrintHexChar(data, blockSize);
        data += blockSize;
    }
}
