#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "convert.h"
#include "typetalk.h"
#include "ruleset.h"

TypeTalk::TypeTalk(Converter *aConverter)
{
    converter = aConverter;
	inputBuffer = NULL;
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
		    ModeTimer = false;
			break;
		case 0x18:
		    // reset
			break;
    }
}

void TypeTalk::putCharacter(char ch)
{
}

void TypeTalk::bufferCharacter(char ch)
{   
	if (ModeEcho) {
		putCharacter(ch);
	}

	if (inputBuffer == NULL) {
		// buffering not supported
		handleCharacter(ch);
		return;
	}

	if ((ch == 0x0A) && (lastInputChar == 0x0D)) {
		// ignore the LF that follows a CR
		return;
	}

	lastInputChar = ch;

	if ((ch == 0x0D) || (ch == 0x0A)) {
		processLine();
		return;
	}

	// add character to buffer
	*inputBufPtr = ch;
	inputBufPtr++;
	inputBufLen++;

	// buffer full ?
	if (inputBufLen >= inputBufSize) {
		processLine();
	}
}

void TypeTalk::processLine() {
	char *x;

    converter->setModePSend(ModePSend);
	converter->addPhoneme(0x03);

    x=inputBuffer;
	while (inputBufLen > 0) {
		handleCharacter(*x);
		x++;
		inputBufLen--;
	}
	handleCharacter(0x0D); // end of line

	inputBufPtr = inputBuffer;
	inputBufLen = 0;
}

void TypeTalk::handleCharacter(char ch) {
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
				converter->convertBuffer();
				*wordBuf = '\0';
				State = STATE_INITIAL;
				goto restart;
			}
			break;

		case STATE_NONLETTER:
		    if (isalpha(ch) || isspace(ch) || (ch=='~')) {
				converter->convertBuffer();
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
	ModeTimer = true;
	wordBuf = converter->getWordBuf();
	lastInputChar = '\0';
}

void TypeTalk::initBuffer(char *bufPtr, int bufLen)
{
	inputBuffer = bufPtr;
	inputBufPtr = inputBuffer;
	inputBufLen = 0;
	inputBufSize = bufLen;
}