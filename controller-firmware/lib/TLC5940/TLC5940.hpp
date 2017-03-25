#ifndef TLC5940_H
#define TLC5940_H

#include "mbed.h"
//#include <FastPWM.h>

 /*

    ASCII art cat(why not?):
         /\_/\
    ____/ o o \
  /~____  =ø= /
 (______)__m_m)

 */

/**
  * SPI speed used by the mbed to communicate with the TLC5940
  * The TLC5940 supports up to 30Mhz. This should be kept as high
  * as possible to ensure that data has time to be sent each reset cycle.
  */
//#define SPI_SPEED 30000000
//#define SPI_SPEED 30000000
#define SPI_SPEED 1000000

/**
  * The rate at which the GSCLK pin is pulsed
  * This also controls how often the reset function is called
  * The rate at which the reset function is called can be calculated by: (1/GSCLK_SPEED) * 4096
  * The maximum reliable rate is around ~32Mhz. I reccomend keeping this as low as possible because
  * a higher rate will use more CPU. Also, this must be low enough to give time for sending new data
  * before the completion of a GSCLK cycle (4096 pulses). If you are daisy chaining multiple TLC5940s,
  * divide 32Mhz by the number of chips to get a good maximum rate.
  */
// #define GSCLK_SPEED 2500000 // does not work with normal PwmOut
// #define GSCLK_SPEED 500000
// #define GSCLK_SPEED 625000 // 4 times slower
// #define GSCLK_SPEED 204800 // 50hZ

/**
  *  This class controls a TLC5940 PWM driver IC.
  *  It supports sending dot correction and grayscale data. However, it does not support error checking or writing the EEPROM.
  *  This class uses the FastPWM library by Erik Olieman to continuously pulse the GSLCK pin without CPU intervention. After
  *  4096 pulses, the private member funciton reset is called by the ticker. It resets the display by pulsing the BLANK pin. If new
  *  data has been set to be sent by the functions setNewGSData or setNewDCData, it is sent here. The definition GSCLK_SPEED in TLC5940.h
  *  controls how often this function is called. A higher GSCLK_SPEED will increase the rate at which the screen is updated but also increase
  *  CPU time spent in that function. The default value is 1Mhz. The rate at which the reset function is called can be calculated by:
  *  (1/GSCLK_SPEED) * 4096.
  *
  *  Using the TLC5940 class to control an LED:
  *  @code
  *  #include "mbed.h"
  *  #include "TLC5940.h"
  *
  *  // Create the TLC5940 instance
  *  TLC5940 tlc(p7, p5, p21, p9, p10, p11, p12, 1);
  *
  *  int main()
  *  {
  *      // Create a buffer to store the data to be sent
  *      unsigned short GSData[16] = { 0x0000 };
  *
  *      // Enable the first LED
  *      GSData[0] = 0xFFF;
  *
  *      // Set the new data
  *      tlc.setNewGSData(GSData);
  *
  *      while(1)
  *      {
  *
  *      }
  *  }
  *  @endcode
  */
class TLC5940
{
public:
    /**
      *  Set up the TLC5940
      *  @param SCLK - The SCK pin of the SPI bus
      *  @param MOSI - The MOSI pin of the SPI bus
      *  @param GSCLK - The GSCLK pin of the TLC5940(s)
      *  @param BLANK - The BLANK pin of the TLC5940(s)
      *  @param XLAT - The XLAT pin of the TLC5940(s)
      *  @param XERR - The XERR pin of the TLC5940(s)
      *  @param number - The number of TLC5940s (if you are daisy chaining)
      */
    TLC5940(PinName SCLK, PinName MOSI, PinName GSCLK, PinName BLANK,
            PinName XLAT, PinName XERR, const int number = 1);

    virtual ~TLC5940() {}

    /**
      *  Set the next chunk of grayscale data to be sent
      *  @param data - Array of 16 bit shorts containing 16 12 bit grayscale data chunks per TLC5940
      *  @note These must be in intervals of at least (1/GSCLK_SPEED) * 4096 to be sent
      */
    void setNewGSData(unsigned short* data);

    /**
      *  Set the next chunk of dot correction data to be sent
      *  @param data - Array of 8 bit chars containing 16 6 bit dot correction data chunks per TLC5940
      *  @note These must be in intervals of at least (1/GSCLK_SPEED) * 4096 to be sent. Also, this function is optional. If you do not
      *  use it, then the TLC5940 will use the EEPROM, which (by default) conatins the data 0x3F.
      */
    void setNewDCData(unsigned char* data);

    /**
     * Returns whether error from the XERR pin is detected.
     */
    bool isErrorDetected();

protected:
    /**
      *  Set the next chunk of grayscale data to be sent while in the current reset cycle
      *  @note This is useful to send the next set of data right after the first is finished being displayed.
      *  The primary purpose for this is multiplexing, although it could be used for anything else.
      */
    virtual void setNextData() {}


    // Number of TLC5940s in series
    const int number;

private:
    // SPI port - only MOSI and SCK are used
    SPI spi;

    // PWM output using the FastPWM library by Erik Olieman
    //FastPWM gsclk;
    PwmOut gsclk;

    // Digital out pins used for the TLC5940
    DigitalOut blank;
    DigitalOut xlat;
    DigitalIn xerr;

    // Call a reset function to manage sending data and GSCLK updating
    Ticker reset_ticker;

    // Has new GS/DC data been loaded?
    volatile bool newGSData;

    // Do we need to send an XLAT pulse? (Was GS data clocked in last reset?)
    volatile bool need_xlat;

    // Buffers to store data until it is sent
    unsigned short* gsBuffer;

    // Function to reset the display and send the next chunks of data
    void reset();

    // Called if an error occurs / is cleared
    void handleErrorDetected();
    void handleErrorCleared();
};

#endif
