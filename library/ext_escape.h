#ifndef __ext_escape_h__
#define __ext_escape_h__

#include "tt_types.h"

#define ESC_PSEND_ON    0
#define ESC_PSEND_OFF   1
#define ESC_ECHO_ON     2
#define ESC_ECHO_OFF    3
#define ESC_CAPS_ON     4
#define ESC_CAPS_OFF    5
#define ESC_TIMER_ON    6
#define ESC_TIMER_OFF   7
#define ESC_RESET       8
#define ESC_CRLF_ON     9
#define ESC_CRLF_OFF   10
#define ESC_PING       11
#define ESC_DAISY      12
#define ESC_VOL        0xFE
#define ESC_NONE       0xFF

extern int escapeParam;
uint8_t reverseMatchExtendedEscape(char *bufPtr, int bufLen, uint8_t *matchLen);

#endif
