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

#ifndef __SDRAM_H
#define __SDRAM_H

#include "stdint.h"

/*
 * These timing parameters are based on the EMC clock
 * there is no way of ensuring what the EMC clock frequency is
 * without severely bloating the code
 * ENSURE THAT THE EMC clock is one of these values
 */
#define SDRAM_SPEED_48 0
#define SDRAM_SPEED_50 1
#define SDRAM_SPEED_60 2
#define SDRAM_SPEED_72 3
#define SDRAM_SPEED_80 4

#define SDRAM_SPEED SDRAM_SPEED_60

#define SDRAM_CONFIG_32BIT
#define SDRAM_SIZE               0x2000000

#define SDRAM_BASE               0xA0000000 /*CS0*/

/* Initializes the SDRAM.
 *
 * The entire SDRAM will be made available to malloc per default.
 *
 * Note that this functions is called internally if malloc requests
 * memory from SDRAM and that hasn't been disabled with a call to
 * sdram_disableMallocSdram().
 *
 * @returns 0 on success, 1 on failure
 */
uint32_t sdram_init();

/* Prevents malloc from using SDRAM.
 *
 * This function must be called before the first allocation that 
 * would have been in SDRAM. If a big allocation has already been
 * made then this call will do nothing as the SDRAM will have been
 * initialized and all SDRAM given to malloc.
 */
void sdram_disableMallocSdram();

#endif /* end __SDRAM_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/


