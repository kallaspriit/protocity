/*
 *  Copyright 2013 Embedded Artists AB
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


/******************************************************************************
 * Includes
 *****************************************************************************/

#include "mbed.h"
#include "sdram.h"

#if defined(TOOLCHAIN_ARM) /* KEIL uVision and mbed online compiler */
#include "sys_helper.h"
#endif

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/


/******************************************************************************
 * External global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

static volatile uint32_t ringosccount[2] = {0,0};

static bool okToUseSdramForHeap = true;
static bool initialized = false;

/******************************************************************************
 * Overridden Global Functions
 *****************************************************************************/

#if defined(TOOLCHAIN_ARM) /* KEIL uVision and mbed online compiler */
  //http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0349c/Cihehbce.html

  extern "C" unsigned __rt_heap_extend(unsigned size, void **block) {
    static uint32_t lastReturnedBlock = 0;

    if (okToUseSdramForHeap && !initialized) {
      sdram_init();
    }

    // Make sure that SDRAM is only returned once (as all of it is returned
    // the first time) and only if the user has chosen to do it (via the
    // okToUseSdramForHeap variable.
    if (okToUseSdramForHeap && lastReturnedBlock==0) {
      *block = (void*)SDRAM_BASE;
      lastReturnedBlock = SDRAM_BASE;
      return SDRAM_SIZE;
    }
    return 0;
  }

  // Overrides the WEAK function in sys_helper.cpp to allow reserving a specific
  // amount of memory for the stack. Without this function it is possible to allocate
  // so much of the internal RAM that there is no free memory for the stack which
  // in turn causes the program to crash.
  uint32_t __reserved_stack_size() {
    return 0x3000; // Reserve 0x3000 bytes of the IRAM for the stack
  }

//#elif defined(TOOLCHAIN_GCC_CR) /* CodeRed's RedSuite or LPCXpresso IDE */
#else
  // NOTE: This way of overriding the implementation of malloc in NEWLIB
  //       will prevent the internal RAM from being used by malloc as
  //       it only exposes the SDRAM.

  // Dynamic memory allocation related syscall.
  extern "C" caddr_t _sbrk(int incr) {
    static unsigned char* heap = (unsigned char*)SDRAM_BASE;
    unsigned char*        prev_heap = heap;
    unsigned char*        new_heap = heap + incr;

    if (okToUseSdramForHeap && !initialized) {
      sdram_init();
    }
    if (!okToUseSdramForHeap) {
      //errno = ENOMEM;
      return (caddr_t)-1;
    }
    if (new_heap >= (unsigned char*)(SDRAM_BASE + SDRAM_SIZE)) {
      //errno = ENOMEM;
      return (caddr_t)-1;
    }

    heap = new_heap;
    return (caddr_t) prev_heap;
  }
#endif

/******************************************************************************
 * Local Functions
 *****************************************************************************/

static void pinConfig(void)
{
  LPC_IOCON->P3_0 |= 1; /* D0 @ P3.0 */
  LPC_IOCON->P3_1 |= 1; /* D1 @ P3.1 */
  LPC_IOCON->P3_2 |= 1; /* D2 @ P3.2 */
  LPC_IOCON->P3_3 |= 1; /* D3 @ P3.3 */

  LPC_IOCON->P3_4 |= 1; /* D4 @ P3.4 */
  LPC_IOCON->P3_5 |= 1; /* D5 @ P3.5 */
  LPC_IOCON->P3_6 |= 1; /* D6 @ P3.6 */
  LPC_IOCON->P3_7 |= 1; /* D7 @ P3.7 */

  LPC_IOCON->P3_8 |= 1; /* D8 @ P3.8 */
  LPC_IOCON->P3_9 |= 1; /* D9 @ P3.9 */
  LPC_IOCON->P3_10 |= 1; /* D10 @ P3.10 */
  LPC_IOCON->P3_11 |= 1; /* D11 @ P3.11 */

  LPC_IOCON->P3_12 |= 1; /* D12 @ P3.12 */
  LPC_IOCON->P3_13 |= 1; /* D13 @ P3.13 */
  LPC_IOCON->P3_14 |= 1; /* D14 @ P3.14 */
  LPC_IOCON->P3_15 |= 1; /* D15 @ P3.15 */

  LPC_IOCON->P3_16 |= 1; /* D16 @ P3.16 */
  LPC_IOCON->P3_17 |= 1; /* D17 @ P3.17 */
  LPC_IOCON->P3_18 |= 1; /* D18 @ P3.18 */
  LPC_IOCON->P3_19 |= 1; /* D19 @ P3.19 */

  LPC_IOCON->P3_20 |= 1; /* D20 @ P3.20 */
  LPC_IOCON->P3_21 |= 1; /* D21 @ P3.21 */
  LPC_IOCON->P3_22 |= 1; /* D22 @ P3.22 */
  LPC_IOCON->P3_23 |= 1; /* D23 @ P3.23 */

  LPC_IOCON->P3_24 |= 1; /* D24 @ P3.24 */
  LPC_IOCON->P3_25 |= 1; /* D25 @ P3.25 */
  LPC_IOCON->P3_26 |= 1; /* D26 @ P3.26 */
  LPC_IOCON->P3_27 |= 1; /* D27 @ P3.27 */

  LPC_IOCON->P3_28 |= 1; /* D28 @ P3.28 */
  LPC_IOCON->P3_29 |= 1; /* D29 @ P3.29 */
  LPC_IOCON->P3_30 |= 1; /* D30 @ P3.30 */
  LPC_IOCON->P3_31 |= 1; /* D31 @ P3.31 */

  LPC_IOCON->P4_0 |= 1; /* A0 @ P4.0 */
  LPC_IOCON->P4_1 |= 1; /* A1 @ P4.1 */
  LPC_IOCON->P4_2 |= 1; /* A2 @ P4.2 */
  LPC_IOCON->P4_3 |= 1; /* A3 @ P4.3 */

  LPC_IOCON->P4_4 |= 1; /* A4 @ P4.4 */
  LPC_IOCON->P4_5 |= 1; /* A5 @ P4.5 */
  LPC_IOCON->P4_6 |= 1; /* A6 @ P4.6 */
  LPC_IOCON->P4_7 |= 1; /* A7 @ P4.7 */

  LPC_IOCON->P4_8 |= 1; /* A8 @ P4.8 */
  LPC_IOCON->P4_9 |= 1; /* A9 @ P4.9 */
  LPC_IOCON->P4_10 |= 1; /* A10 @ P4.10 */
  LPC_IOCON->P4_11 |= 1; /* A11 @ P4.11 */

  LPC_IOCON->P4_12 |= 1; /* A12 @ P4.12 */
  LPC_IOCON->P4_13 |= 1; /* A13 @ P4.13 */
  LPC_IOCON->P4_14 |= 1; /* A14 @ P4.14 */
#if 0 // not used for SDRAM
  LPC_IOCON->P4_15 |= 1; /* A15 @ P4.15 */

  LPC_IOCON->P4_16 |= 1; /* A16 @ P4.16 */
  LPC_IOCON->P4_17 |= 1; /* A17 @ P4.17 */
  LPC_IOCON->P4_18 |= 1; /* A18 @ P4.18 */
  LPC_IOCON->P4_19 |= 1; /* A19 @ P4.19 */

  LPC_IOCON->P4_20 |= 1; /* A20 @ P4.20 */
  LPC_IOCON->P4_21 |= 1; /* A21 @ P4.21 */
  LPC_IOCON->P4_22 |= 1; /* A22 @ P4.22 */
  LPC_IOCON->P4_23 |= 1; /* A23 @ P4.23 */
#endif

  LPC_IOCON->P4_24 |= 1; /* OEN @ P4.24 */
  LPC_IOCON->P4_25 |= 1; /* WEN @ P4.25 */
#if 0 // not used for SDRAM
  LPC_IOCON->P4_26 |= 1; /* BLSN[0] @ P4.26 */
  LPC_IOCON->P4_27 |= 1; /* BLSN[1] @ P4.27 */


  LPC_IOCON->P4_28 |= 1; /* BLSN[2] @ P4.28 */
  LPC_IOCON->P4_29 |= 1; /* BLSN[3] @ P4.29 */
  LPC_IOCON->P4_30 |= 1; /* CSN[0] @ P4.30 */
  LPC_IOCON->P4_31 |= 1; /* CSN[1] @ P4.31 */
#endif

  LPC_IOCON->P2_14 |= 1; /* CSN[2] @ P2.14 */
  LPC_IOCON->P2_15 |= 1; /* CSN[3] @ P2.15 */

  LPC_IOCON->P2_16 |= 1; /* CASN @ P2.16 */
  LPC_IOCON->P2_17 |= 1; /* RASN @ P2.17 */
  LPC_IOCON->P2_18 |= 1; /* CLK[0] @ P2.18 */
#if 0 // not used for SDRAM
  LPC_IOCON->P2_19 |= 1; /* CLK[1] @ P2.19 */
#endif

  LPC_IOCON->P2_20 |= 1; /* DYCSN[0] @ P2.20 */
#if 0 // not used for SDRAM
  LPC_IOCON->P2_21 |= 1; /* DYCSN[1] @ P2.21 */
  LPC_IOCON->P2_22 |= 1; /* DYCSN[2] @ P2.22 */
  LPC_IOCON->P2_23 |= 1; /* DYCSN[3] @ P2.23 */
#endif

  LPC_IOCON->P2_24 |= 1; /* CKE[0] @ P2.24 */
#if 0 // not used for SDRAM
  LPC_IOCON->P2_25 |= 1; /* CKE[1] @ P2.25 */
  LPC_IOCON->P2_26 |= 1; /* CKE[2] @ P2.26 */
  LPC_IOCON->P2_27 |= 1; /* CKE[3] @ P2.27 */
#endif

  LPC_IOCON->P2_28 |= 1; /* DQM[0] @ P2.28 */
  LPC_IOCON->P2_29 |= 1; /* DQM[1] @ P2.29 */
  LPC_IOCON->P2_30 |= 1; /* DQM[2] @ P2.30 */
  LPC_IOCON->P2_31 |= 1; /* DQM[3] @ P2.31 */
}


static uint32_t sdram_test( void )
{
  volatile uint32_t *wr_ptr;
  volatile uint16_t *short_wr_ptr;
  uint32_t data;
  uint32_t i, j;

  wr_ptr = (uint32_t *)SDRAM_BASE;
  short_wr_ptr = (uint16_t *)wr_ptr;
  /* Clear content before 16 bit access test */
//  for (i = 0; i < SDRAM_SIZE/4; i++)
//  {
//	*wr_ptr++ = 0;
//  }

  /* 16 bit write */
  for (i = 0; i < SDRAM_SIZE/0x40000; i++)
  {
    for (j = 0; j < 0x100; j++)
    {
      *short_wr_ptr++ = (i + j);
      *short_wr_ptr++ = (i + j) + 1;
    }
  }

  /* Verifying */
  wr_ptr = (uint32_t *)SDRAM_BASE;
  for (i = 0; i < SDRAM_SIZE/0x40000; i++)
  {
    for (j = 0; j < 0x100; j++)
    {
      data = *wr_ptr;
      if (data != (((((i + j) + 1) & 0xFFFF) << 16) | ((i + j) & 0xFFFF)))
      {
        return 0x0;
      }
      wr_ptr++;
    }
  }
  return 0x1;
}

static uint32_t find_cmddly(void)
{
  uint32_t cmddly, cmddlystart, cmddlyend, dwtemp;
  uint32_t ppass = 0x0, pass = 0x0;

  cmddly = 0x0;
  cmddlystart = cmddlyend = 0xFF;

  while (cmddly < 32)
  {
    dwtemp = LPC_SC->EMCDLYCTL & ~0x1F;
    LPC_SC->EMCDLYCTL = dwtemp | cmddly;

    if (sdram_test() == 0x1)
    {
      /* Test passed */
      if (cmddlystart == 0xFF)
      {
        cmddlystart = cmddly;
      }
      ppass = 0x1;
    }
    else
    {
      /* Test failed */
      if (ppass == 1)
      {
        cmddlyend = cmddly;
        pass = 0x1;
        ppass = 0x0;
      }
    }

    /* Try next value */
    cmddly++;
  }

  /* If the test passed, the we can use the average of the min and max values to get an optimal DQSIN delay */
  if (pass == 0x1)
  {
    cmddly = (cmddlystart + cmddlyend) / 2;
  }
  else if (ppass == 0x1)
  {
    cmddly = (cmddlystart + 0x1F) / 2;
  }
  else
  {
    /* A working value couldn't be found, just pick something safe so the system doesn't become unstable */
    cmddly = 0x10;
  }

  dwtemp = LPC_SC->EMCDLYCTL & ~0x1F;
  LPC_SC->EMCDLYCTL = dwtemp | cmddly;

  return (pass | ppass);
}

static uint32_t find_fbclkdly(void)
{
  uint32_t fbclkdly, fbclkdlystart, fbclkdlyend, dwtemp;
  uint32_t ppass = 0x0, pass = 0x0;

  fbclkdly = 0x0;
  fbclkdlystart = fbclkdlyend = 0xFF;

  while (fbclkdly < 32)
  {
    dwtemp = LPC_SC->EMCDLYCTL & ~0x1F00;
    LPC_SC->EMCDLYCTL = dwtemp | (fbclkdly << 8);

    if (sdram_test() == 0x1)
    {
      /* Test passed */
      if (fbclkdlystart == 0xFF)
      {
        fbclkdlystart = fbclkdly;
      }
      ppass = 0x1;
    }
    else
    {
      /* Test failed */
      if (ppass == 1)
      {
        fbclkdlyend = fbclkdly;
        pass = 0x1;
        ppass = 0x0;
      }
    }

    /* Try next value */
    fbclkdly++;
  }

  /* If the test passed, the we can use the average of the min and max values to get an optimal DQSIN delay */
  if (pass == 0x1)
  {
    fbclkdly = (fbclkdlystart + fbclkdlyend) / 2;
  }
  else if (ppass == 0x1)
  {
    fbclkdly = (fbclkdlystart + 0x1F) / 2;
  }
  else
  {
    /* A working value couldn't be found, just pick something safe so the system doesn't become unstable */
    fbclkdly = 0x10;
  }

  dwtemp = LPC_SC->EMCDLYCTL & ~0x1F00;
  LPC_SC->EMCDLYCTL = dwtemp | (fbclkdly << 8);

  return (pass | ppass);
}

static uint32_t calibration( void )
{
  uint32_t dwtemp, i;
  uint32_t cnt = 0;

  for (i = 0; i < 10; i++)
  {
    dwtemp = LPC_SC->EMCCAL & ~0x4000;
    LPC_SC->EMCCAL = dwtemp | 0x4000;

    dwtemp = LPC_SC->EMCCAL;
    while ((dwtemp & 0x8000) == 0x0000)
    {
      dwtemp = LPC_SC->EMCCAL;
    }
    cnt += (dwtemp & 0xFF);
  }
  return (cnt / 10);
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/


void adjust_timing( void )
{
  uint32_t dwtemp, cmddly, fbclkdly;

  /* Current value */
  ringosccount[1] = calibration();

  dwtemp   = LPC_SC->EMCDLYCTL;
  cmddly   = ((dwtemp & 0x1F) * ringosccount[0] / ringosccount[1]) & 0x1F;
  fbclkdly = ((dwtemp & 0x1F00) * ringosccount[0] / ringosccount[1]) & 0x1F00;
  LPC_SC->EMCDLYCTL = (dwtemp & ~0x1F1F) | fbclkdly | cmddly;
}

/******************************************************************************
 *
 * Description:
 *    Initialize the SDRAM
 *
 *****************************************************************************/
uint32_t sdram_init (void)
{
  uint32_t i;
  uint32_t dwtemp = 0;
  //uint16_t wtemp = 0;

  if (initialized) {
    return 0;
  }

  LPC_SC->PCONP     |= 0x00000800;
  LPC_SC->EMCDLYCTL  = 0x00001010;
  LPC_EMC->Control   = 0x00000001;
  LPC_EMC->Config    = 0x00000000;

  pinConfig(); //Full 32-bit Data bus, 24-bit Address

  /* Configure memory layout, but MUST DISABLE BUFFERs during configuration */
  /* 256MB, 8Mx32, 4 banks, row=12, column=9 */
  LPC_EMC->DynamicConfig0    = 0x00004480;

  /*Configure timing for ISSI IS4x32800D SDRAM*/

#if (SDRAM_SPEED==SDRAM_SPEED_48)
//Timing for 48MHz Bus
  LPC_EMC->DynamicRasCas0    = 0x00000201; /* 1 RAS, 2 CAS latency */
  LPC_EMC->DynamicReadConfig = 0x00000001; /* Command delayed strategy, using EMCCLKDELAY */
  LPC_EMC->DynamicRP         = 0x00000000; /* ( n + 1 ) -> 1 clock cycles */
  LPC_EMC->DynamicRAS        = 0x00000002; /* ( n + 1 ) -> 3 clock cycles */
  LPC_EMC->DynamicSREX       = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicAPR        = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicDAL        = 0x00000002; /* ( n ) -> 2 clock cycles */
  LPC_EMC->DynamicWR         = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicRC         = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicRFC        = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicXSR        = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicRRD        = 0x00000000; /* ( n + 1 ) -> 1 clock cycles */
  LPC_EMC->DynamicMRD        = 0x00000000; /* ( n + 1 ) -> 1 clock cycles */
#elif (SDRAM_SPEED==SDRAM_SPEED_50)
//Timing for 50MHz Bus (with 100MHz M3 Core)
  LPC_EMC->DynamicRasCas0    = 0x00000201; /* 1 RAS, 2 CAS latency */
  LPC_EMC->DynamicReadConfig = 0x00000001; /* Command delayed strategy, using EMCCLKDELAY */
  LPC_EMC->DynamicRP         = 0x00000000; /* ( n + 1 ) -> 1 clock cycles */
  LPC_EMC->DynamicRAS        = 0x00000002; /* ( n + 1 ) -> 3 clock cycles */
  LPC_EMC->DynamicSREX       = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicAPR        = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicDAL        = 0x00000002; /* ( n ) -> 2 clock cycles */
  LPC_EMC->DynamicWR         = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicRC         = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicRFC        = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicXSR        = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicRRD        = 0x00000000; /* ( n + 1 ) -> 1 clock cycles */
  LPC_EMC->DynamicMRD        = 0x00000000; /* ( n + 1 ) -> 1 clock cycles */
#elif (SDRAM_SPEED==SDRAM_SPEED_60)
  //Timing for 60 MHz Bus (same as 72MHz)
  LPC_EMC->DynamicRasCas0    = 0x00000202; /* 2 RAS, 2 CAS latency */
  LPC_EMC->DynamicReadConfig = 0x00000001; /* Command delayed strategy, using EMCCLKDELAY */
  LPC_EMC->DynamicRP         = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicRAS        = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicSREX       = 0x00000005; /* ( n + 1 ) -> 6 clock cycles */
  LPC_EMC->DynamicAPR        = 0x00000002; /* ( n + 1 ) -> 3 clock cycles */
  LPC_EMC->DynamicDAL        = 0x00000003; /* ( n ) -> 3 clock cycles */
  LPC_EMC->DynamicWR         = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicRC         = 0x00000004; /* ( n + 1 ) -> 5 clock cycles */
  LPC_EMC->DynamicRFC        = 0x00000004; /* ( n + 1 ) -> 5 clock cycles */
  LPC_EMC->DynamicXSR        = 0x00000005; /* ( n + 1 ) -> 6 clock cycles */
  LPC_EMC->DynamicRRD        = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicMRD        = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
#elif (SDRAM_SPEED==SDRAM_SPEED_72)
  //Timing for 72 MHz Bus
  LPC_EMC->DynamicRasCas0    = 0x00000202; /* 2 RAS, 2 CAS latency */
  LPC_EMC->DynamicReadConfig = 0x00000001; /* Command delayed strategy, using EMCCLKDELAY */
  LPC_EMC->DynamicRP         = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicRAS        = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicSREX       = 0x00000005; /* ( n + 1 ) -> 6 clock cycles */
  LPC_EMC->DynamicAPR        = 0x00000002; /* ( n + 1 ) -> 3 clock cycles */
  LPC_EMC->DynamicDAL        = 0x00000003; /* ( n ) -> 3 clock cycles */
  LPC_EMC->DynamicWR         = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicRC         = 0x00000004; /* ( n + 1 ) -> 5 clock cycles */
  LPC_EMC->DynamicRFC        = 0x00000004; /* ( n + 1 ) -> 5 clock cycles */
  LPC_EMC->DynamicXSR        = 0x00000005; /* ( n + 1 ) -> 6 clock cycles */
  LPC_EMC->DynamicRRD        = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicMRD        = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
#elif (SDRAM_SPEED==SDRAM_SPEED_80)
  //Timing for 80 MHz Bus (same as 72MHz)
  LPC_EMC->DynamicRasCas0    = 0x00000202; /* 2 RAS, 2 CAS latency */
  LPC_EMC->DynamicReadConfig = 0x00000001; /* Command delayed strategy, using EMCCLKDELAY */
  LPC_EMC->DynamicRP         = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicRAS        = 0x00000003; /* ( n + 1 ) -> 4 clock cycles */
  LPC_EMC->DynamicSREX       = 0x00000005; /* ( n + 1 ) -> 6 clock cycles */
  LPC_EMC->DynamicAPR        = 0x00000002; /* ( n + 1 ) -> 3 clock cycles */
  LPC_EMC->DynamicDAL        = 0x00000003; /* ( n ) -> 3 clock cycles */
  LPC_EMC->DynamicWR         = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicRC         = 0x00000004; /* ( n + 1 ) -> 5 clock cycles */
  LPC_EMC->DynamicRFC        = 0x00000004; /* ( n + 1 ) -> 5 clock cycles */
  LPC_EMC->DynamicXSR        = 0x00000005; /* ( n + 1 ) -> 6 clock cycles */
  LPC_EMC->DynamicRRD        = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
  LPC_EMC->DynamicMRD        = 0x00000001; /* ( n + 1 ) -> 2 clock cycles */
#else
	#error UNSUPPORTED SDRAM FREQ
#endif

  LPC_EMC->DynamicControl    = 0x00000183; /* Issue NOP command */
  wait(0.2);                         /* wait 200ms */
  LPC_EMC->DynamicControl    = 0x00000103; /* Issue PALL command */
  LPC_EMC->DynamicRefresh    = 0x00000002; /* ( n * 16 ) -> 32 clock cycles */
  for(i = 0; i < 0x80; i++);               /* wait 128 AHB clock cycles */


#if (SDRAM_SPEED==SDRAM_SPEED_48)
  //Timing for 48MHz Bus
  LPC_EMC->DynamicRefresh    = 0x0000002E; /* ( n * 16 ) -> 736 clock cycles -> 15.330uS at 48MHz <= 15.625uS ( 64ms / 4096 row ) */
#elif (SDRAM_SPEED==SDRAM_SPEED_50)
  //Timing for 50MHz Bus
  LPC_EMC->DynamicRefresh    = 0x0000003A; /* ( n * 16 ) -> 768 clock cycles -> 15.360uS at 50MHz <= 15.625uS ( 64ms / 4096 row ) */
#elif (SDRAM_SPEED==SDRAM_SPEED_60)
  //Timing for 60MHz Bus
  LPC_EMC->DynamicRefresh    = 0x0000003A; /* ( n * 16 ) -> 928 clock cycles -> 15.466uS at 60MHz <= 15.625uS ( 64ms / 4096 row ) */
#elif (SDRAM_SPEED==SDRAM_SPEED_72)
  //Timing for 72MHz Bus
  LPC_EMC->DynamicRefresh    = 0x00000046; /* ( n * 16 ) -> 1120 clock cycles -> 15.556uS at 72MHz <= 15.625uS ( 64ms / 4096 row ) */
#elif (SDRAM_SPEED==SDRAM_SPEED_80)
  //Timing for 80MHz Bus
  LPC_EMC->DynamicRefresh    = 0x0000004E; /* ( n * 16 ) -> 1248 clock cycles -> 15.600uS at 80MHz <= 15.625uS ( 64ms / 4096 row ) */
#else
	#error UNSUPPORTED SDRAM FREQ
#endif

  LPC_EMC->DynamicControl    = 0x00000083; /* Issue MODE command */
  //Timing for 48/60/72MHZ Bus
  dwtemp = *((volatile uint32_t *)(SDRAM_BASE | (0x22<<(2+2+9)))); /* 4 burst, 2 CAS latency */
  dwtemp = dwtemp;
  LPC_EMC->DynamicControl    = 0x00000000; /* Issue NORMAL command */
//[re]enable buffers
  LPC_EMC->DynamicConfig0    = 0x00084480; /* 256MB, 8Mx32, 4 banks, row=12, column=9 */

  /* Nominal value */
  ringosccount[0] = calibration();

  if (find_cmddly() == 0x0)
  {
    //while (1);  /* fatal error */
    return 1;//FALSE;
  }

  if (find_fbclkdly() == 0x0)
  {
    //while (1);  /* fatal error */
    return 1;//FALSE;
  }

  adjust_timing();

  initialized = true;

  return 0;//TRUE;
}

void sdram_disableMallocSdram()
{
  okToUseSdramForHeap = false;
}
