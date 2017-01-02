#include "mbed.h"
#include "TLC5940.hpp"


TLC5940Mux::TLC5940Mux(PinName SCLK, PinName MOSI, PinName GSCLK, PinName BLANK,
                       PinName XLAT, PinName DCPRG, PinName VPRG, const int number,
                       const int rows, void (*SetRows)(int)) : TLC5940(SCLK, MOSI, GSCLK, BLANK, XLAT, DCPRG, VPRG, number),
                                                               rows(rows),
                                                               SetRows(SetRows),
                                                               currentIndex(0)

{
    // Create a data buffer to store the current LED states
    dataBuffer = new unsigned short[rows * 16 * number];

    // Zero the buffer
    memset(dataBuffer, 0x00, rows * 16 * number * 2);
}

TLC5940Mux::~TLC5940Mux()
{
    // Delete the buffer
    delete[] dataBuffer;
}

unsigned short* TLC5940Mux::operator=(unsigned short* data)
{
    // Copy the memory from data to the data buffer
    memcpy(dataBuffer, data, rows * 16 * number * 2);

    return data;
}

unsigned short* TLC5940Mux::operator[](int index)
{
    // Return the start of the correct data chunk
    return dataBuffer + (index * 16 * number);
}

void TLC5940Mux::setNextData()
{
    // Move into the dataBuffer and return a pointer corresponding to the start of the correct data block
    setNewGSData(dataBuffer + (currentIndex * 16 * number));

    // Since the data we sent on the previous reset was just latched in,
    // we must enable the row for the previous index so it is displayed in the right position
    // The ternary is used in case index is zero (we can't have an index of -1)
    SetRows((currentIndex ? (currentIndex - 1) : (rows - 1)));

    // Go to next index
    if (currentIndex == (rows - 1))
        currentIndex = 0;
    else
        currentIndex++;
}
