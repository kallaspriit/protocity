#include "TLC5940.hpp"

#include <algorithm>

TLC5940::TLC5940(PinName mosi, PinName miso, PinName sck, PinName xlat, PinName blank, PinName gsclk)
{
    _spi = new SPI(mosi, miso, sck);
    _xlat = new DigitalOut(xlat);
    _blank = new DigitalOut(blank);
    _gsclk = new PwmOut(gsclk);
    _t = new Ticker();

    // Setup SPI
    _spi->format(12,0);
    //_spi->frequency(5000000);
    _spi->frequency(30000000);

    //Turn off GSCLK
    _gsclk->write(0.0f);
    _gsclk->period_us(5);

    // Reset to 0
    for(int i = 0; i < 16; i++)
    {
           gs_data[i] = 4095;
           int whoami = _spi->write(4095);
    }

    _xlat->write(1);
    _xlat->write(0);
}

void TLC5940::setValue(int channel, float value)
{
    int exactValue = min(max((int)(value * 4095.0f), 0), 4095);

    setExactValue(channel, exactValue);
}

void TLC5940::setExactValue(int channel, int value)
{
    gs_data[15-channel] = value;
}

void TLC5940::flush()
{
    for(int i = 0; i < 16; i++){
        _spi->write(gs_data[i]);
    }

    _xlat->write(1);
    _xlat->write(0);
}

void TLC5940::run()
{
    const double GSCLK_SPEED = 2500000.0;

    _gsclk->write(0.5f);
    //_t->attach_us(this, &TLC5940::refresh, 5*4096);
    _t->attach_us(this, &TLC5940::refresh, (1000000.0/GSCLK_SPEED) * 4096.0);
}

void TLC5940::refresh()
{
    _blank->write(1);
    _blank->write(0);
}
