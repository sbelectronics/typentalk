#ifndef __TT_TYPES_H__
#define __TT_TYPES_H__

#ifdef ARDUINO

#include <Arduino.h>

#define RULEMEM PROGMEM

#else

/* On any reasonably modern hosted target (Linux, Pico SDK, etc.) use the
 * standard headers — defining uint8_t / true / false ourselves conflicts
 * with stdint.h / stdbool.h when any other code in the same TU pulls them
 * in. Pre-C99 / Z8000 zcc builds can fall back via -DLEGACY_TYPES. */
#ifdef LEGACY_TYPES
typedef unsigned char uint8_t;
#define false 0
#define true 1
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#define RULEMEM

#endif

#endif
