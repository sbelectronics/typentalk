#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "convert_stdio.h"
#include "phoneme.h"

void StdioConverter::SetEmit(bool b)
{
	ModeEmit = b;
}

void StdioConverter::emitPhoneme(uint8_t phoneme)
{
	if (ModeEmit) {
		char phonemeBuf[MAX_PHONEME_BUF];

		if (getPhoneme(phoneme & 0x3F, phonemeBuf)) {
            fprintf(stdout, "<%s> ", phonemeBuf);
		}
	}
}

void StdioConverter::putCharacter(char ch)
{
	putchar(ch);
}

void StdioConverter::DebugPrintf(const char *format, ...)
{
    if (!debug) {
		return;
	}

	va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);

	va_end(args);
}

void StdioConverter::DebugRule(const char *txt, uint8_t *myRulePtr, uint8_t *graPtr)
{
    uint8_t *save;
	char phonemeBuf[MAX_PHONEME_BUF];

    if (!debug) {
		return;
	}	

	DebugPrintf("%s", txt);
	while ((*myRulePtr & 0x80) != 0) {
		DebugPrintf(" %02X", *myRulePtr);
        myRulePtr++;
	}

	save = myRulePtr;

	DebugPrintf(" %02X", *myRulePtr);

	// increment rule pointer
	myRulePtr++;

    // loop while b7 unset (skip phonemes)
	while ((*myRulePtr & 0x80) == 0) {
		DebugPrintf(" %02X", *myRulePtr);
		myRulePtr++;
	}

    myRulePtr = save;
	DebugPrintf(" (");
	getPhoneme(*myRulePtr & 0x3F, phonemeBuf);
	DebugPrintf("/%s", phonemeBuf);
	myRulePtr++;
	while ((*myRulePtr & 0x80) == 0) {
		getPhoneme(*myRulePtr & 0x3F, phonemeBuf);
		DebugPrintf("/%s", phonemeBuf);
		myRulePtr++;
	}
	DebugPrintf(")");

	if (graPtr) {
		DebugPrintf(" [%02X]", *graPtr);
	}

	DebugPrintf("\n");
}