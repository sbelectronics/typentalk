#ifndef __TT_TYPES_H__
#define __TT_TYPES_H__

#ifdef ARDUINO

#include <Arduino.h>

#define RULEMEM PROGMEM

#else

typedef unsigned char uint8_t;
#define false 0
#define true 1
#define RULEMEM

#endif

#endif
