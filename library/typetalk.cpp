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

void TypeTalk::executeEscape(uint8_t escapeOp)
{
	switch (escapeOp) {
	    case ESC_PSEND_ON :
    		ModePSend = true;
	    	break;
	    case ESC_PSEND_OFF:
    		ModePSend = false;
			break;
		case ESC_ECHO_ON:
		    ModeEcho = true;
			break;
		case ESC_ECHO_OFF:
		    ModeEcho = false;
			break;
		case ESC_CAPS_ON:
		    ModeCaps = true;
			break;
		case ESC_CAPS_OFF:
		    ModeCaps = false;
			break;
		case ESC_TIMER_OFF:
		    ModeTimer = false;
			break;
		case ESC_CRLF_ON:
		    ModeCRLF = true;
			break;
		case ESC_CRLF_OFF:
		    ModeCRLF = false;
			break;
		case ESC_PING:
		    putCharacter('P');
			putCharacter('O');
			putCharacter('N');
			putCharacter('G');
			break;
    }
}

void TypeTalk::handleEscapeCharacter(char ch)
{
	switch (ch) {
	    case 0x11:
		    executeEscape(ESC_PSEND_ON);
	    	break;
	    case 0x12:
		    executeEscape(ESC_PSEND_OFF);
			break;
		case 0x13:
		    executeEscape(ESC_ECHO_ON);
			break;
		case 0x14:
		    executeEscape(ESC_ECHO_OFF);
			break;
		case 0x15:
		    executeEscape(ESC_CAPS_ON);
			break;
		case 0x16:
		    executeEscape(ESC_CAPS_OFF);
			break;
		case 0x17:
		    executeEscape(ESC_TIMER_OFF);
			break;
		case 0x18:
		    executeEscape(ESC_RESET);
			break;
    }
}

void TypeTalk::putCharacter(char ch)
{
}

void TypeTalk::bufferCharacter(char ch)
{   
    uint8_t escSeq, escLen;

	if (ModeEcho) {
		putCharacter(ch);
	}

	if (ch == '\0') {
		return;
	}

	if (inputBuffer == NULL) {
		// buffering not supported
		handleCharacter(ch);
		return;
	}

	if (lastInputChar == 0x18) {
		// we're in an escape sequence
        handleEscapeCharacter(ch);
		lastInputChar = ch;
		return;
	}

	if ((ch == 0x0A) && (lastInputChar == 0x0D)) {
		// ignore the LF that follows a CR
		lastInputChar = ch;
		return;
	}

	lastInputChar = ch;

	if (ch == 0x08) {
		// backspace
        if (inputBufLen>0) {
			inputBufLen--;
			inputBufPtr--;
		}
		return;
	}

	if ((ch == 0x0D) || (ch == 0x0A)) {
		processLine();
		return;
	}

	// add character to buffer
	*inputBufPtr = ch;
	inputBufPtr++;
	inputBufLen++;

	// extended escape sequence
	escSeq = (reverseMatchExtendedEscape(inputBufPtr, inputBufLen, &escLen));
	if (escSeq != ESC_NONE) {
        inputBufPtr -= escLen;
		inputBufLen -= escLen;
		executeEscape(escSeq);
	}

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
				} else {
					State = STATE_NONLETTER;
					goto restart;
				}
			}
			
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
	ModeCRLF = false;
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