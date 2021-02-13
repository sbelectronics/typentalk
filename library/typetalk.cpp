#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "convert.h"
#include "typetalk.h"
#include "ruleset.h"

TypeTalk::TypeTalk(Converter *aConverter)
{
    converter = aConverter;
    init();
}

void TypeTalk::handleEscape(char ch)
{
	switch (ch) {
	    case 0x11:
    		ModePSend = true;
	    	break;
	    case 0x12:
    		ModePSend = false;
			break;
		case 0x13:
		    ModeEcho = true;
			break;
		case 0x14:
		    ModeEcho = false;
			break;
		case 0x15:
		    ModeCaps = true;
			break;
		case 0x16:
		    ModeCaps = false;
			break;
		case 0x17:
		    // timer off
			break;
		case 0x18:
		    // reset
			break;
    }
}

void TypeTalk::putCharacter(char ch)
{
}

void TypeTalk::handleCharacter(char ch) {
	if (ModeEcho) {
		putCharacter(ch);
	}
  restart:
	switch (State) {
		case STATE_INITIAL:
			if (State == STATE_INITIAL) {
				if (isalpha(ch)) {
					State = STATE_LETTER;
					goto restart;
				} else if (isspace(ch)) {
					State = STATE_SPACE;
					goto restart;
				} else if (ch == '~') {
					// Phoneme sequence is ~ ... ?
                    State = STATE_PHONEME;
					goto restart;
				} else if (ch == 0x1B) {
					State = STATE_ESCAPE;
					// no restart -- fall through and return
				} else {
					State = STATE_NONLETTER;
					goto restart;
				}
			}

		case STATE_ESCAPE:
		    handleEscape(ch);
			State = STATE_INITIAL;
			// no restart -- fall through and return
		    break;
			
		case STATE_SPACE:
		    State = STATE_INITIAL;
			// no restart -- fall through and return
		    break;

		case STATE_PHONEME:
		    if (ch == '?') {
				State = STATE_INITIAL;
				// no restart -- fall through and return
			} else {
                converter->emitPhoneme(ch & 0x3F);
			}
		    break;

		case STATE_LETTER:
			if (isalpha(ch)) {
				strncat(wordBuf, &ch, 1);
			} else {
				converter->setModePSend(ModePSend);
				converter->convertString(wordBuf);
				*wordBuf = '\0';
				State = STATE_INITIAL;
				goto restart;
			}
			break;

		case STATE_NONLETTER:
		    if (isalpha(ch) || isspace(ch) || (ch=='~')) {
				converter->convertString(wordBuf);
				*wordBuf = '\0';
				State = STATE_INITIAL;
				goto restart;
			} else {
				strncat(wordBuf, &ch, 1);
			}
	}
}

void TypeTalk::init()
{
	State = STATE_INITIAL;
	ModePSend = false;
	ModeEcho = true;
    ModeCaps = false;
	*wordBuf = '\0';
}