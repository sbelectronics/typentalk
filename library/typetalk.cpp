/*
 * Type 'N Talk Text to Speech Engine
 * 
 * Scott Baker, http://www.smbaker.com/
 * 
 * This is the Votrax Type 'N Talk text to speech engine, converted from
 * an assembly language source originally disassembled by Simon Rowe. This
 * is not a machine-generated conversion, but represents hand conversion
 * of the assembly behavior into the C++ language, by Scott Baker. It is
 * not pretty code -- I did not understand 6800 assembly well when I 
 * did the conversion, and I have not been a C/C++ programmer for a
 * decade so my skills are rusty. It's a hodgepodge of C/C++ syntax
 * together with assembly workflow.
 * 
 * It is, however, reasonably correct. Probably not perfect, but is
 * close.
 * 
 * typetalk.cpp is the high-level parsing engine that buffers text until
 * a CR is pressed, then breaks up the text into a series of words and
 * converts then individually.
 */

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
		case ESC_RESET:
		    init();
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
			echoCharacter(0x00);
	    	break;
	    case 0x12:
		    executeEscape(ESC_PSEND_OFF);
			echoCharacter(0x00);
			break;
		case 0x13:
		    executeEscape(ESC_ECHO_ON);
			echoCharacter(0x00);
			break;
		case 0x14:
		    executeEscape(ESC_ECHO_OFF);
			echoCharacter(0x00);
			break;
		case 0x15:
		    executeEscape(ESC_CAPS_ON);
			echoCharacter(0x00);
			break;
		case 0x16:
		    executeEscape(ESC_CAPS_OFF);
			echoCharacter(0x00);
			break;
		case 0x17:
		    executeEscape(ESC_TIMER_OFF);
			echoCharacter(0x00);
			break;
		case 0x18:
		    echoCharacter(0x00);
		    executeEscape(ESC_RESET);
			break;
		case 0x1B:
		    echoCharacter(0x1B);
			break;			
    }
}

void TypeTalk::echoCharacter(char ch)
{
	if (ModeEcho) {
		putCharacter(ch);
		if ((ch=='\r') && (ModeCRLF)) {
			putCharacter('\n');
		}
	}
}

void TypeTalk::putCharacter(char ch)
{
}

void TypeTalk::bufferCharacter(char ch)
{   
    uint8_t escSeq, escLen;

	if (inEscapeSequence) {
		// we're in an escape sequence
        handleEscapeCharacter(ch);
		inEscapeSequence = false;
		return;
	}

	echoCharacter(ch);

	if (ch == '\0') {
		return;
	}

	if (inputBuffer == NULL) {
		// buffering not supported
		handleCharacter(ch);
		return;
	}

	if ((ch == 0x0A) && (lastInputChar == 0x0D)) {
		// ignore the LF that follows a CR
		lastInputChar = ch;
		return;
	}

	lastInputChar = ch;

	if (ch == 0x1B) {
		// escape
		inEscapeSequence = true;
		return;
	}

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
				if (strlen(wordBuf) >= 0x1B) {
					// Overflow case is weird. We say the word, but we don't consume
					// the final character. Instead, we use it for the start of the
					// next word.
				    converter->setModePSend(ModePSend);
				    converter->setModeCaps(ModeCaps);
				    converter->convertBuffer();
				    *wordBuf = '\0';
					strncat(wordBuf, &ch, 1);
				}
			} else {
				converter->setModePSend(ModePSend);
				converter->setModeCaps(ModeCaps);
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
	inEscapeSequence = false;
}

void TypeTalk::initBuffer(char *bufPtr, int bufLen)
{
	inputBuffer = bufPtr;
	inputBufPtr = inputBuffer;
	inputBufLen = 0;
	inputBufSize = bufLen;
}