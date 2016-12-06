
/*
Copyright (c) 2010 Donatien Garnier (donatiengar [at] gmail [dot] com)
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef TMP102_H
#define TMP102_H

#include "mbed.h"

//!Library for the TI TMP102 temperature sensor.
/*!
The TMP102 is an I2C digital temperature sensor in a small SOT563 package, with a 0.0625C resolution and 0.5C accuracy.
*/
class TMP102
{
public:
  //!Creates an instance of the class.
  /*!
  Connect module at I2C address addr using I2C port pins sda and scl.
  TMP102
  \param addr <table><tr><th>A0 pin connection</th><th>Address</th></tr><tr><td>GND</td><td>0x90</td></tr><tr><td>V+</td><td>0x92</td></tr><tr><td>SDA</td><td>0x94</td></tr><tr><td>SCL</td><td>0x96</td></tr></table>  
  */
  TMP102(PinName sda, PinName scl, int addr);
  
  /*!
  Destroys instance.
  */ 
  ~TMP102();
  
  //!Reads the current temperature.
  /*!
  Reads the temperature register of the TMP102 and converts it to a useable value.
  */
  float read();
  
private:
  I2C m_i2c;
  int m_addr;

};

#endif
