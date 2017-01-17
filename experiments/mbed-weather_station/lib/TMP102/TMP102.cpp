
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

#include "TMP102.h"

#define TEMP_REG_ADDR 0x00

TMP102::TMP102(PinName sda, PinName scl, int addr) : m_i2c(sda, scl), m_addr(addr)
{

}

TMP102::~TMP102()
{

}

float TMP102::read()
{
  
  const char tempRegAddr = TEMP_REG_ADDR;

  m_i2c.write(m_addr, &tempRegAddr, 1); //Pointer to the temperature register
 
  char reg[2] = {0,0};
  m_i2c.read(m_addr, reg, 2); //Rea
  
  int16_t res  = ((int8_t)reg[0] << 4) | ((uint8_t)reg[1] >> 4);  
  
  float temp =  (float) ((float)res * 0.0625);
   
  return temp;
}
