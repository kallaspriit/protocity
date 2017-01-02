#ifndef TLC5940_H_
#define TLC5940_H_

#include "mbed.h"

class TLC5940 {
private:
    SPI * _spi;
    PwmOut * _gsclk;
    DigitalOut * _xlat;
    DigitalOut * _blank;
    Ticker * _t;
    int gs_data[16];
public:
    TLC5940(PinName mosi, PinName miso, PinName sck, PinName xlat, PinName blank, PinName gsclk);
    void setValue(int channel, float value); // 0.0f ... 1.0f
    void setExactValue(int channel, int value); // 0 ... 4095
    void flush();
    void run();
    void refresh();
};

#endif /* TLC5940_H_ */
