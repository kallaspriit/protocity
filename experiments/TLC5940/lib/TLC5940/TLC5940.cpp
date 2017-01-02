#include "TLC5940.hpp"

TLC5940::TLC5940(PinName SCLK, PinName MOSI, PinName GSCLK, PinName BLANK,
                 PinName XLAT, PinName DCPRG, PinName VPRG, const int number) : number(number),
                                                                                spi(MOSI, NC, SCLK),
                                                                                gsclk(GSCLK),
                                                                                blank(BLANK),
                                                                                xlat(XLAT),
                                                                                dcprg(DCPRG),
                                                                                vprg(VPRG),
                                                                                newGSData(false),
                                                                                newDCData(false),
                                                                                need_xlat(false)
{
    // Configure SPI to 12 bits and SPI_SPEED
    spi.format(12, 0);
    spi.frequency(SPI_SPEED);

    // Set output pin states
    dcprg = 0;
    vprg = 0;
    xlat = 0;
    blank = 1;

    // Call the reset function every 4096 PWM outputs
    reset_ticker.attach_us(this, &TLC5940::reset, (1000000.0/GSCLK_SPEED) * 4096.0);

    // Configure FastPWM output for GSCLK frequency at 50% duty cycle
    //gsclk.period_us(1000000.0/(GSCLK_SPEED * 1.05));
    gsclk.period_us(1);
    gsclk.write(.5);
}

void TLC5940::setNewGSData(unsigned short* data)
{
    gsBuffer = data;

    // Tell reset function that new GS data has been given
    newGSData = true;
}

void TLC5940::setNewDCData(unsigned char* data)
{
    dcBuffer = data;

    // Tell reset function that new DC data has been given
    newDCData = true;
}

void TLC5940::reset()
{
    gsclk.write(0);
    // Turn off LEDs
    blank = 1;

    // Virtual function that allows the next data chunk to be set after every GSCLK cycle
    // Useful for setting the next frame when multiplexing (e.g. LED matrices)
    setNextData();

    // Latch in data from previous cycle if needed
    if (need_xlat)
    {
        // Latch
        xlat = 1;
        xlat = 0;

        // Don't need to latch again
        need_xlat = false;
    }

    // Reset the screen so that it is updating while data is being sent
    blank = 0;
    gsclk.write(.5);

    // Do we have new DC data to send?
    if (newDCData)
    {
        // Set TLC5940 to accpet DC data
        vprg = 1;

        // Get DC data from registers instead of EEPROM (since we are sending data to the registers now)
        dcprg = 1;

        // Send DC data backwards - this makes the DC_buffer[0] index correspond to OUT0
        for (int i = (16 * number) - 1; i >= 0; i--)
        {
            // Assemble a 12 bit packet from two 6 bit chunks
            spi.write(((dcBuffer[i] & 0x3F) << 6) | (dcBuffer[i-1] & 0x3F));
            i--;
        }

        // Latch
        xlat = 1;
        xlat = 0;

        // No new data to send (we just sent it!)
        newDCData = false;
    }

    // Do we have new GS data to send?
    if (newGSData)
    {
        // Set TLC5940 to accept GS data
        vprg = 0;

        // Send GS data backwards - this makes the GS_buffer[0] index correspond to OUT0
        for (int i = (16 * number) - 1; i >= 0; i--)
        {
            // Get the lower 12 bits of the buffer and send
            spi.write(gsBuffer[i] & 0xFFF);
        }

        // Latch after current GS data is done being displayed
        need_xlat = true;

        // No new data to send (we just sent it!)
        newGSData = false;
    }
}
