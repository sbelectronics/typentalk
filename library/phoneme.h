#ifndef __PHONEME_H__
#define __PHONEME_H__

#include "tt_types.h"

// "STOP" plus the null terminator
#define MAX_PHONEME_BUF 5

bool getPhoneme(uint8_t phoneme, char *dest);

#endif
