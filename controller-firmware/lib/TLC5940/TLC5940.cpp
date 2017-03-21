#include "TLC5940.hpp"

TLC5940::TLC5940(PinName SCLK, PinName MOSI, PinName GSCLK, PinName BLANK,
                 PinName XLAT, PinName XERR, const int number) : number(number),
                                                                                spi(MOSI, NC, SCLK),
                                                                                gsclk(GSCLK),
                                                                                blank(BLANK),
                                                                                xlat(XLAT),
                                                                                xerr(XERR),
                                                                                newGSData(false),
                                                                                need_xlat(false)
{
    // Configure SPI to 12 bits and SPI_SPEED
    spi.format(12, 0);
    spi.frequency(SPI_SPEED);

    // Set output pin states
    xlat = 0;
    blank = 1;

    // Puppup the error detection pin
    xerr.mode(PullUp);

    // Call the reset function every 4096 PWM outputs
    // reset_ticker.attach_us(callback(this, &TLC5940::reset), (1000000.0/GSCLK_SPEED) * 4096.0);
    reset_ticker.attach_us(callback(this, &TLC5940::reset), 20 * 1000); // 20 ms = 50hZ

    // Configure FastPWM output for GSCLK frequency at 50% duty cycle
    //gsclk.period_us(1000000.0/(GSCLK_SPEED * 1.05)); // FastPWM 0.381.. at 2500000 hz
    // gsclk.period_us(1); // minimum possible <
    gsclk.period_us(4); // < 4.8828125
    gsclk.write(.5);
}

void TLC5940::setNewGSData(unsigned short* data)
{
    gsBuffer = data;

    // Tell reset function that new GS data has been given
    newGSData = true;
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

    // Do we have new GS data to send?
    if (newGSData)
    {
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

bool TLC5940::isErrorDetected() {
    // return xerr.read() == 0 ? true : false;
    return false;
}
