#include <stdlib.h>
#include <string.h>
#include "ext_escape.h"

#ifdef ARDUINO
#define ESCMEM PROGMEM
#else
#define ESCMEM
#endif

const char esc_psend_on[] ESCMEM = "{{psendon}}";
const char esc_psend_off[] ESCMEM = "{{psendoff}}";
const char esc_echo_on[] ESCMEM = "{{echoon}}";
const char esc_echo_off[] ESCMEM = "{{echooff}}";
const char esc_caps_on[] ESCMEM = "{{capson}}";
const char esc_caps_off[] ESCMEM = "{{capsoff}}";
const char esc_timer_on[] ESCMEM = "{{timeron}}";
const char esc_timer_off[] ESCMEM = "{{timeroff}}";
const char esc_reset[] ESCMEM = "{{reset}}";
const char esc_crlf_on[] ESCMEM = "{{crlfon}}";
const char esc_crlf_off[] ESCMEM = "{{crlfoff}}";
const char esc_ping[] ESCMEM = "{{ping}}";
const char esc_daisy[] ESCMEM = "{{daisy}}";

const char *const esc_table[] ESCMEM = {
	esc_psend_on,
	esc_psend_off,
	esc_echo_on,
	esc_echo_off,
	esc_caps_on,
	esc_caps_off,
    esc_timer_on,
    esc_timer_off,
    esc_reset,
	esc_crlf_on,
	esc_crlf_off,
    esc_ping,
    esc_daisy,
    NULL};

int escapeParam;

uint8_t reverseMatchExtendedEscape(char *bufPtr, int bufLen, uint8_t *matchLen)
{
    int i = 0;

    if (bufLen<4) {
        // if less than four characters, can't be anything
        return ESC_NONE;
    }

    if (((*(bufPtr-1)) != '}') || ((*(bufPtr-2)) != '}'))  {
        // escape sequences end in }}
        return ESC_NONE;
    }

    // volume command: {{volNN}}
    if ((bufLen>=9) && (strncasecmp(bufPtr-9, "{{vol", 5) == 0)) {
        char tmp[3];
        strncpy(tmp, bufPtr-4, 2);
        tmp[2] = '\0';
        escapeParam = atoi(tmp); 
        *matchLen = 9;
        return ESC_VOL;
    }

#ifdef ARDUINO
    for (i=0; ; i++) {
        char *eseq = (char *) pgm_read_word(&(esc_table[i]));
        int len;

        if (eseq == NULL) {
            // nothing more to check
            return ESC_NONE;
        }

        len = strlen_P(eseq);
        if (len > bufLen) {
            // the buffer isn't long enough
            continue;
        }

        if (strncasecmp_P(bufPtr-len, eseq, len) == 0) {
            *matchLen = len;
            return i;
        }
    }
#else
    for (i=0; ; i++) {
        char *eseq = (char *) esc_table[i];
        int len;

        if (eseq == NULL) {
            // nothing more to check
            return ESC_NONE;
        }

        len = strlen(eseq);
        if (len > bufLen) {
            // the buffer isn't long enough
            continue;
        }

        if (strncasecmp(bufPtr-len, eseq, len) == 0) {
            *matchLen = len;
            return i;
        }
    }
#endif
}



