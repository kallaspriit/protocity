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
#define SPI_SPEED 30000000

/**
  * The rate at which the GSCLK pin is pulsed
  * This also controls how often the reset function is called
  * The rate at which the reset function is called can be calculated by: (1/GSCLK_SPEED) * 4096
  * The maximum reliable rate is around ~32Mhz. I reccomend keeping this as low as possible because
  * a higher rate will use more CPU. Also, this must be low enough to give time for sending new data
  * before the completion of a GSCLK cycle (4096 pulses). If you are daisy chaining multiple TLC5940s,
  * divide 32Mhz by the number of chips to get a good maximum rate.
  */
#define GSCLK_SPEED 2500000

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
      *  @param VPRG - The VPRG pin of the TLC5940(s)
      *  @param number - The number of TLC5940s (if you are daisy chaining)
      */
    TLC5940(PinName SCLK, PinName MOSI, PinName GSCLK, PinName BLANK,
            PinName XLAT, PinName VPRG, const int number = 1);

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
    DigitalOut vprg;

    // Call a reset function to manage sending data and GSCLK updating
    Ticker reset_ticker;

    // Has new GS/DC data been loaded?
    volatile bool newGSData;
    volatile bool newDCData;

    // Do we need to send an XLAT pulse? (Was GS data clocked in last reset?)
    volatile bool need_xlat;

    // Buffers to store data until it is sent
    unsigned short* gsBuffer;
    unsigned char* dcBuffer;

    // Function to reset the display and send the next chunks of data
    void reset();
};


/**
  *  This class allows a TLC5940 to be multiplexed.
  *  It inherits the TLC5940 class and uses it to control the TLC5940 driver(s). It does not support sending dot corection data.
  *  This class sets the new grayscale data every iteration of the GSCLK reset loop. It then updates the current row using the
  *  user defined function SetRows. The framerate you will recieve using this function can be calculate by: 1 / (((1/GSCLK_SPEED) * 4096) * rows).
  *  I reccomend maintaining a framerate above 30fps. However, keep in mind that as your framerate increases, so does your CPU usage.
  *
  *  Using the TLC5940Mux class to control an 8x8 LED matrix:
  *  @code
  *  #include "mbed.h"
  *  #include "TLC5940.h"
  *
  *  // Bus connecting to the rows of the LED matrix through PNP transistors
  *  BusOut rows(p22, p23, p24, p25, p26, p27, p28, p29);
  *
  *  // Function to update the rows using the BusOut class
  *  void SetRows(int nextRow)
  *  {
  *      // I am using PNP transistors, so inversion is necessary
  *      rows = ~(1 << nextRow);
  *  }
  *
  *  // Create the TLC5940Mux instance
  *  TLC5940Mux tlc(p7, p5, p21, p9, p10, p11, p12, 1, 8, &SetRows);
  *
  *  int main()
  *  {
  *      tlc[0][0] = 0xFFF; // Turn on the top left LED
  *      while(1)
  *      {
  *
  *      }
  *  }
  *  @endcode
  */
class TLC5940Mux : private TLC5940
{
public:
    /**
      *  Set up the TLC5940
      *  @param SCLK - The SCK pin of the SPI bus
      *  @param MOSI - The MOSI pin of the SPI bus
      *  @param GSCLK - The GSCLK pin of the TLC5940(s)
      *  @param BLANK - The BLANK pin of the TLC5940(s)
      *  @param XLAT - The XLAT pin of the TLC5940(s)
      *  @param VPRG - The VPRG pin of the TLC5940(s)
      *  @param number - The number of TLC5940s (if you are daisy chaining)
      *  @param rows - The number of rows you are multiplexing
      *  @param SetRows - The function pointer to your function that sets the current row.
      *  @note The SetRows function allows you to set exactly how you want your rows
      *  to be updated. The TLC5940Mux class calls this function with an argument of int that contains the number of the row to
      *  be turned on. If the TLC5940Mux class needs the first row to be turned on, the int will be 0.
      */
    TLC5940Mux(PinName SCLK, PinName MOSI, PinName GSCLK, PinName BLANK,
               PinName XLAT, PinName VPRG, const int number,
               const int rows, void (*SetRows)(int));

    // Destructor used to delete memory
    ~TLC5940Mux();

    /**
      *  Set the contents of the buffer that contains the multiplexed data
      *  @param data - The data to set to the buffer containing 16 12 bit grayscale data chunks per TLC5940
      *  @returns The data provided
      */
    unsigned short* operator=(unsigned short* data);

    /**
      *  Get a pointer to one of the rows of the multiplexed data
      *  @param index - The row that you would like the contents of
      *  @returns A pointer to the data containing the requested row containing 16 12 bit grayscale data chunks per TLC5940
      *  @note This operator can also be used to change or get the value of an individual LED.
      *  For example:
      *  @code
      *  TLC5940Mux[0][0] = 0xFFF;
      *  @endcode
      */
    unsigned short* operator[](int index);

private:
    // Virtual function overriden from TLC5940 class
    virtual void setNextData();

    // Number of rows
    const int rows;

    // Function to set the current row
    void (*SetRows)(int);

    // The current row
    volatile int currentIndex;

    // Buffer containing data to be sent during each frame
    unsigned short* dataBuffer;
};

#endif
