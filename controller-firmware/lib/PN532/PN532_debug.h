#ifndef __PN532_DEBUG_H__
#define __PN532_DEBUG_H__

//#define DEBUG
//#define MIFARE_ULTRALIGHT_DEBUG
//#define MIFARE_CLASSIC_DEBUG
//#define NDEF_DEBUG

#ifdef DEBUG

#include <stdio.h>

#define DMSG(args...)       printf(args)

#else

#define DMSG(args...)

#endif

#endif
