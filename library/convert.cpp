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
 * convert.cpp is the low-level conversion engine. It implements the
 * text to speech rules (see ruleset.cpp). Each rule is a left context
 * and a right context together with a set of phonemes to emit if the
 * rule matches. Rules are evaluated in sequence until a matching
 * rule is found. 
 * 
 * For the high-level code that splits sentences into words, interprets
 * escape sequences, etc., see typetalk.cpp.
 */

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "convert.h"
#include "ruleset.h"

/*
 *  Rules look like this
 * 
 *  L2 L1 <BF> R1 R2 [Phon] [Phon] ... [Phon]
 *
 *  Rule bytes have the high bit set.
 *  Phoneme bytes do not have the high bit set.
 * 
 *  For example,
 * 
 *     E0 FB BF DA 7F
 *       E0 match multi voewl
 *       FB match zero or more consonants
 *       BF context separator
 *       DA match punctuation
 *       7F emit stop
 */

// read a rule byte
#ifdef ARDUINO
#define RRB(p) (pgm_read_byte_near(p))
#define DEBUGPRINT(fmt)
#define DEBUGPRINTF(fmt, ...)
#define DEBUGRULE(text, rulePtr, graPtr)
#else
#define RRB(p) (*(p))
#define DEBUGPRINT(fmt) DebugPrintf(fmt)
#define DEBUGPRINTF(fmt, args...) DebugPrintf(fmt, args)
#define DEBUGRULE(text, rulePtr, graPtr) DebugRule(text, rulePtr, graPtr)
#endif

// tests for bit6 and bit7 of a rulePtr
#define RULEBIT6(p) ((RRB(p) & 0x40)!=0)
#define NORULEBIT6(p) ((RRB(p) & 0x40)==0)
#define RULEBIT7(p) ((RRB(p) & 0x80)!=0)
#define NORULEBIT7(p) ((RRB(p) & 0x80)==0)

const unsigned char *ruleSets[] = {
	ruleSetY,	// grapheme $00
	ruleSetE,	// grapheme $01
	ruleSetI,	// grapheme $02
	ruleSetO,	// grapheme $03
	ruleSetU,	// grapheme $04
	ruleSetA,	// grapheme $05
	ruleSetF,	// grapheme $06
	ruleSetK,	// grapheme $07
	ruleSetP,	// grapheme $08
	ruleSetQ,	// grapheme $09
	ruleSetH,	// grapheme $0A
	ruleSetT,	// grapheme $0B
	ruleSetS,	// grapheme $0C
	ruleSetC,	// grapheme $0D
	ruleSetX,	// grapheme $0E
	ruleSetG,	// grapheme $0F
	ruleSetJ,	// grapheme $10
	ruleSetZ,	// grapheme $11
	ruleSetR,	// grapheme $12
	ruleSetD,	// grapheme $13
	ruleSetL,	// grapheme $14
	ruleSetN,	// grapheme $15
	ruleSetB,	// grapheme $16
	ruleSetV,	// grapheme $17
	ruleSetM,	// grapheme $18
	ruleSetW,	// grapheme $19
	ruleSetSym	// numeric/symbol grapheme $1A
};

int txt2grTable[] = {
	0x1A,		// numeric/symbol
	0x05,		// 'A'
	0x16,		// 'B'
	0x0D,		// 'C'
	0x13,		// 'D'
	0x01,		// 'E'
	0x06,		// 'F'
	0x0F,		// 'G'
	0x0A,		// 'H'
	0x02,		// 'I'
	0x10,		// 'J'
	0x07,		// 'K'
	0x14,		// 'L'
	0x18,		// 'M'
	0x15,		// 'N'
	0x03,		// 'O'
	0x08,		// 'P'
	0x09,		// 'Q'
	0x12,		// 'R'
	0x0C,		// 'S'
	0x0B,		// 'T'
	0x04,		// 'U'
	0x17,		// 'V'
	0x19,		// 'W'
	0x0E,		// 'X'
	0x00,		// 'Y'
	0x11,		// 'Z'
	// no separate table for these graphemes, use $1A
	0x1B,		// '['
	0x1C,		// '\'
	0x1D,		// ']'
	0x1E,		// '^'
	0x1F		// '_'
};

Converter::Converter()
{
    debug = false;
	modePSend = false;
	modeCaps = false;
}

void Converter::setModePSend(bool b)
{
	modePSend = b;
}

void Converter::setModeCaps(bool b)
{
	modeCaps = b;
}

char *Converter::getWordBuf()
{
    uint8_t *graphenes = grapheneBuf;

    // sentinels at start of word
    *graphenes = 0xFF; // sentinel
	graphenes++;
	*graphenes = 0x7F; // sentinel
	graphenes++;

	*graphenes = 0;

	return (char*) graphenes;
}

// Convert a word into a list of graphenes.
uint8_t Converter::wordToGraphenesInPlace(uint8_t *graphenes)
{
    uint8_t count = 0;

	while (*graphenes) {
        if (*graphenes < 0x40) {
			// space, punctuation, numbers, or control characters
			// copy as-is
			graphenes++;
			count++;
        } else {
            uint8_t index = tolower(*graphenes) - 'a' + 1;
			DEBUGPRINTF("XXX %02X %02X %02X\n", *graphenes, index, txt2grTable[index]);
            *graphenes = txt2grTable[index];
            graphenes++;
            count++;
        }
    }

    // two sentinels at end of word
	*graphenes = 0xFF;
	graphenes++;
	*graphenes = 0xFF;
	graphenes++;

    return count;
}

const uint8_t *Converter::getRuleSet(uint8_t grapheme)
{
	if (grapheme>0x1A) {
		grapheme = 0x1A;
	}
	DEBUGPRINTF("getRuleSet grapheme=%02X\n", grapheme);
	return ruleSets[grapheme];
}

void Converter::convertBuffer()
{
	uint8_t len;
	bool letterByLetter = false;

	if (modeCaps && (isupper(*(grapheneBuf+2)) && isupper(*(grapheneBuf + 3)))) {
		letterByLetter = true;
	}

    len = wordToGraphenesInPlace(grapheneBuf + 2);

    if (letterByLetter) {
		// for letter-by-letter mode, construct a little buffer that
		// will hold only one character.

		uint8_t buf[5] = {0xFF, 0x7F, 0x00, 0xFF, 0xFF};
		int i;

		for (i=0; i<len; i++) {
			buf[2] = grapheneBuf[i+2];
			convert(buf+2, 1);
		}
	} else {
	    convert(grapheneBuf+2, len); // two-byte sentinel at start
	}
}

void Converter::addPhoneme(uint8_t phoneme)
{
	if (modePSend) {
		putCharacter(phoneme + 0x40);
	} else {
	    emitPhoneme(phoneme);
	}
}

void Converter::convert(uint8_t *graphemeStart, uint8_t count)
{
	graCount = count;
    graPtr = graphemeStart;

	while (graCount > 0) {
	    const uint8_t *ruleSet;
		const uint8_t *nextRule;

        DEBUGPRINT("----------\n");
		ruleSet = getRuleSet(*graPtr);
        rulePtr = ruleSet;
		LocateRightContext(rulePtr);
		while (true) {
			// XXX confusion about *ruleSet passed here?
			if (matchRuleSet()) {
				while ((NORULEBIT7(matchPtr)) && ((RRB(matchPtr)&0x3F) != 0x3F)) {
					addPhoneme(RRB(matchPtr));
					matchPtr++;
				}
				graPtr += graLen;
				graCount -= graLen;
				break;
			}
			
			nextRule = skipRule(rulePtr);
			LocateRightContext(nextRule);

			// assumption: this loop will always terminate in a match
		}
	}
}

void Converter::LocateRightContext(const uint8_t *myRulePtr)
{
    uint8_t grapheme;

getRightContext:
    DEBUGRULE("getRightContext", myRulePtr, graPtr);
	while (RULEBIT6(myRulePtr)) {
		myRulePtr++;
	}

	graLen = 0;
	rightRulePtr =  myRulePtr;
	rulePtr = myRulePtr;
	nextRightGraphemePtr = graPtr;

	grapheme = *graPtr; // XXX suspicious
	if (RRB(rulePtr) == 0xBF) {
		DEBUGPRINT("  context sep / noMoreRules\n");
		goto L6D6A; // context separator
	}

	if (RRB(rulePtr) >= 0x9A) {
		DEBUGPRINT("  numeric/symbol match rule\n");
		goto L6D73; // numeric/symbol match rule
	}

    graLen ++;
	rightGraphemePtr = nextRightGraphemePtr;
	nextRightGraphemePtr = rightGraphemePtr+1;
	myRulePtr = rulePtr;
	goto L6D73;

L6D6A:
    rightGraphemePtr = nextRightGraphemePtr;
	graLen ++;
	DEBUGPRINTF("  rightGraphemePtr=[%02X]\n", *rightGraphemePtr);
	// XXX last instruction before return is inx, and x is rightGraphemePtr
	return;

L6D73:
	if (NORULEBIT7(myRulePtr)) {
		DEBUGPRINT("  L6D73 end of rule\n");
		return;
	}

	if (RULEBIT6(myRulePtr)) {
		DEBUGPRINT("  L6D73 return right context but keep\n");
		// right context but keep, return
		return;
	}

	graLen ++;
	grapheme = *nextRightGraphemePtr;
	if ((grapheme & 0x80) != 0) {
		// encountered the end sentinel on the word
		DEBUGPRINT("  skiprule\n");
		myRulePtr = skipRule(myRulePtr);
		goto getRightContext;
	}

	// note: aslb followed by bs lsrb strips the top bit

	if (grapheme != (RRB(myRulePtr) & 0x7F)) {
		// rule does not match
		DEBUGPRINTF("  skip rule no match <%02X>[%02X]\n", RRB(myRulePtr) & 0x7F, grapheme);
		myRulePtr = skipRule(myRulePtr);
		goto getRightContext;
	}

	rightGraphemePtr = nextRightGraphemePtr;
	nextRightGraphemePtr = rightGraphemePtr+1;
	rulePtr += 1;
	myRulePtr = rulePtr;
	goto L6D73;
}

const uint8_t *Converter::skipRule(const uint8_t *myRulePtr)
{
    //DEBUGPRINTF("skiprule myRulePtr=%p, *myRulePtr=%02X, *myRulePtr+1=%02X\n", myRulePtr, *myRulePtr, *(myRulePtr+1));

    // increment rule pointer
	myRulePtr++;

    // loop while b7 set
	while (RULEBIT7(myRulePtr)) {
        myRulePtr++;
	}

	// increment rule pointer
	myRulePtr++;

    // loop while b7 unset (skip phonemes)
	while (NORULEBIT7(myRulePtr)) {
		myRulePtr++;
	}

	//DEBUGPRINTF("skiprule returning myRulePtr=%p, *myRulePtr=%02X, *myRulePtr+1=%02X\n", myRulePtr, *myRulePtr, *(myRulePtr+1));

	return myRulePtr;
}

bool Converter::matchRuleSet()
{
    const uint8_t *myRulePtr;

	DEBUGRULE("matchRuleSet", rightRulePtr, graPtr);

	graMatchPtr = graPtr;

	myRulePtr = rightRulePtr;

	// left context
L6DA7:
    myRulePtr--;
	DEBUGRULE("matchRuleSet-backup", myRulePtr, graPtr);
	if (NORULEBIT7(myRulePtr)) {
		// end of left context; scan forwards
		goto L6DB9;
	}
	rulePtr = myRulePtr;
	context = 0; // left context
	if (!matchRule(RRB(rulePtr))) {
		DEBUGPRINT("  rule does not match\n");
		return false;
	}
	goto L6DA7;

	// right context
L6DB9:
    DEBUGPRINTF("  scanForward <%02X>[%02X]\n", *rightRulePtr, *rightGraphemePtr);
    graMatchPtr = rightGraphemePtr;
	myRulePtr = rightRulePtr;
L6DBF:
    myRulePtr++;
	DEBUGRULE("matchRuleSet-forward", myRulePtr, graPtr);
	if (NORULEBIT7(myRulePtr)) {
		DEBUGPRINT("  no more rules\n");
		// no more rules, use following phonemes
		matchPtr = myRulePtr;
        return true;
	}
	if (NORULEBIT6(myRulePtr)) {
		DEBUGPRINT("  right context already consumed");
		// skip right context already consumed
		goto L6DBF;
	}
	rulePtr = myRulePtr;
	context = 1; // right context
	if (!matchRule(RRB(myRulePtr))) {
		DEBUGPRINT("  rule does not match\n");
		return false;
	}
	// continue forward
	myRulePtr = rulePtr;
	goto L6DBF;
}

bool Converter::matchRule(uint8_t ruleByte)
{
    if (context == 0) {
		// left context
		graMatchPtr = graMatchPtr-1;
	} else {
		// right context
		graMatchPtr = graMatchPtr+1;
	}
    DEBUGPRINTF("  matchRule <%02X>[%02x] ctx=%d\n", ruleByte, *graMatchPtr, context);

	if ((ruleByte & 0x20) != 0) {
		// bit5 set -> special match rule
		DEBUGPRINT("    matRule-matchSpecial\n");
		return matchSpecial(ruleByte);
	}

	ruleByte = ruleByte & 0x3F; // clear bits 7 and 6

	if (ruleByte <= 0x19) {
		DEBUGPRINTF("    matRule-exit2 %d\n", ruleByte == *graMatchPtr);
		return (ruleByte == *graMatchPtr);
	}

     // numeric/symbol match
	 DEBUGPRINTF("    matchRule-matchsymbol <%02X>[%02X]:%d\n", ruleByte, *graMatchPtr, (ruleByte <= *graMatchPtr));
	 return (ruleByte <= *graMatchPtr);
}

bool Converter::matchSpecial(uint8_t ruleByte)
{
	ruleByte = ruleByte & 0x1F; // lower 5 bits

    // world's shittiest case statement...
	if ((ruleByte & 0x10) == 0) {
		// bit 4 unset
		if (ruleByte == 0) {
			// bits 0-3 unset
			return matchMultiVowel();
		}
		if (ruleByte == 1) {
			// XXX comment seems inconsistent with code here
			// seems like code is testing for b0 unset
            // only bit 0 is set
			return matchFrontVowel();
		}
		return matchNumeric();
	} else {
		// bit 4 set
		if ((ruleByte & 0x08) == 0) {
			// bit 3 clear
			if ((ruleByte & 0x04) == 0) {
				// bit 2 is clear
				return matchSibilant();
			} else {
				// bit 2 is set
                return matchSuffix();
			}
		} else {
			// bit 3 is set
			if ((ruleByte & 0x04) == 0) {
				// bit 2 is clear
				if ((ruleByte & 0x02)!=0) {
					// bit 1 is set
					return matchZeroCons();
				} else {
					// bit 1 is clear
					if ((ruleByte & 0x01)!=0) {
						// bit 0 is set
						return matchVoicedCons();
					} else {
						return matchLongUCons();
				    }
				}
			} else {
				// bit 2 is set
				if ((ruleByte & 0x02) == 0) {
					// bit 1 is clear
				    if ((ruleByte & 0x01) != 0) {
					    // bit 0 is set
					    return matchMultiCons();
				    } else {
						// bit 0 is clear
    					return matchConsonant();
	    			}
			    } else {
					// bit 1 is set
					if ((ruleByte & 0x01) != 0) {
						// bit 0 is set
						return matchNonLetterStuff();
					} else {
						// bit 0 is clear
						return matchNonLetterS();
					}
			    }
			}
		}
	}
}

bool Converter::matchMultiVowel()
{
	uint8_t vowelCount = 0;
	uint8_t *myGraPtr = graMatchPtr;
	uint8_t grapheme;

	DEBUGPRINTF("    matchMultiVowel [%02X]\n", *myGraPtr);

again:
	grapheme = *myGraPtr;
	if ((grapheme & 0x80) != 0) {
		// end of word
		return (vowelCount > 0);
	}
	if (grapheme > 0x05) {
		// greater than "A", is a consonant
		return (vowelCount > 0);
	}

	graMatchPtr = myGraPtr;
	vowelCount++;

	if (context == 0) {
		// left context
		myGraPtr--;
	} else {
		myGraPtr++;
	}
	goto again;
}

bool Converter::matchFrontVowel()
{
	uint8_t grapheme;

	DEBUGPRINTF("    matchFrontVowel [%02X]\n", *graMatchPtr);

	grapheme = *graMatchPtr;
	if ((grapheme & 0x80) != 0) {
		// end of word
        return false;
	}

    // less than or equal to "I"
	return (grapheme <= 0x02);
}

bool Converter::matchMultiCons()
{
	uint8_t consCount = 0;
	uint8_t *myGraPtr = graMatchPtr;
	uint8_t grapheme;

	DEBUGPRINTF( "    matchMultiCons [%02X]\n", *graMatchPtr);

again:
    grapheme = *myGraPtr;
	if ((grapheme & 0x80) != 0) {
		// end of word
		return (consCount > 0);
	}
	if (grapheme <= 0x05) {
		// less than or equal to "A", is a vowel
		return (consCount > 0);
	}
	if (grapheme > 0x19) {
		// greater than "W", is a symbol
		return (consCount > 0);
	}
	graMatchPtr = myGraPtr;
	consCount++;
	if (context == 0) {
		// left context
		myGraPtr--;
	} else {
		myGraPtr++;
	}
	goto again;
}

bool Converter::matchConsonant()
{
	uint8_t grapheme;

	DEBUGPRINTF("    matchConsonant [%02X]\n", *graMatchPtr);

	grapheme = *graMatchPtr;
	if ((grapheme & 0x80) != 0) {
		// end of word
        return false;
	}

    // "F" through "W"
	return ((grapheme >= 0x06) && (grapheme <= 0x19));
}

bool Converter::matchNonLetterStuff()
{
    uint8_t grapheme;

	DEBUGPRINTF("    matchNonLetterStuff [%02X]\n", *graMatchPtr);

	if (matchSuffix()) {
		return true;
	}

	grapheme = *graMatchPtr;
	// greater than "W" is a non-letter
	return (grapheme > 0x19);
}

bool Converter::matchSuffix()
{
	uint8_t *myGraPtr = graMatchPtr;
	uint8_t grapheme;

	DEBUGPRINTF("    matchSuffix [%02X]\n", *graMatchPtr);

    grapheme = *myGraPtr;
	if ((grapheme & 0x80) != 0) {
		return false;
	}

	if (grapheme != 0x01) { // E
	    goto L6ED9;
	}

	myGraPtr++;
	grapheme = *myGraPtr;
	if (grapheme == 0x12) { // R
	    goto L6EC0;
	}
	if (grapheme > 0x19) { // non-letter; word ends in "E"
        graMatchPtr = myGraPtr;
		return true;    
	}
	if (grapheme == 0x13) { // D
	    goto L6EC0;
	}
	if (grapheme != 0x14) { // L
	    goto L6ECC;
	}

	myGraPtr++;
	grapheme = *myGraPtr;
	if ((grapheme & 0x80) != 0) {
		return false;
	}
	if (grapheme == 0) { // Y
	    goto L6EC0;
	}
	return false;

L6EC0:
    myGraPtr++;
	grapheme = *myGraPtr;
	if (grapheme <= 0x19) { // W
	    goto L6ECC;
	}
	// fall through

L6EC7:
    graMatchPtr = myGraPtr;
	return true;

L6ECC:
    if (grapheme != 0x0C) { //S
	    // not S, no match
		return false;
	}
	myGraPtr++;
	grapheme = *myGraPtr;
	if (grapheme > 0x19) {
		// non-letter, we have a match
		goto L6EC7;
	}
	// no match
	return false;

L6ED9:
    if (grapheme != 0x02) { // I
	    return false;
	}
	myGraPtr++;
	grapheme = *myGraPtr;
	if ((grapheme & 0x80) != 0) {
		return false;
	}	
	if (grapheme != 0x15) { // N
	    return false;
	}
	myGraPtr++;
	grapheme = *myGraPtr;
	if ((grapheme & 0x80) != 0) {
		return false;
	}
	if (grapheme != 0x0F) { // G
	   return false;
	}
	// check following grapheme for non-letter
    goto L6EC0;
}

bool Converter::matchSibilant()
{
	uint8_t *myGraPtr = graMatchPtr;
	uint8_t grapheme;

	DEBUGPRINTF("    matchSibilant [%02X]\n", *graMatchPtr);

    grapheme = *myGraPtr;
	if ((grapheme & 0x80) != 0) {
		return false;
	}

	if (grapheme <= 0x0B)	{ //T
	    goto L6F13;
	}
	if (grapheme > 0x0D)    { //C
	    goto L6F0D;
	}
	if (context == 0) {
		// left context, return match
		return true;
	}

	grapheme = *(myGraPtr+1);
	if (grapheme != 0x0A) { // H
		return true;
	}

	myGraPtr++;
	graMatchPtr = myGraPtr;
	return true;

L6F0D:
    if (grapheme <= 0x11) { // Z
        return true;
	}
	return false;

L6F13:
    if (context == 1) {
		// right context, no match
		return false;
	}
	if (grapheme != 0x0A) { // H
	    return false;
	}
	myGraPtr--;
	grapheme = *myGraPtr;
	if (grapheme <= 0x0B) { // T
	    return false;
	}
	if (grapheme > 0x0D) { // C
	    return false;
	}

	// sequence was S or C followed by H
	graMatchPtr = myGraPtr;
	return true;
}

bool Converter::matchZeroCons()
{
    DEBUGPRINTF("    matchZeroCons [%02X]\n", *graMatchPtr);

	if (matchMultiCons()) {
		return true;
	}
	if (context == 0) {
		graMatchPtr++;
		return true;
	} else {
		graMatchPtr--;
		return true;
	}
}

bool Converter::matchVoicedCons()
{
	uint8_t *myGraPtr = graMatchPtr;
	uint8_t grapheme;

	DEBUGPRINTF("    matchVoicedCons [%02X]\n", *graMatchPtr);

    grapheme = *myGraPtr;
	if ((grapheme & 0x80) != 0) {
		return false;
	}

	return ((grapheme > 0x0E) && (grapheme <= 0x19)); // match if X..W
}

bool Converter::matchLongUCons()
{
	uint8_t *myGraPtr = graMatchPtr;
	uint8_t grapheme;

	DEBUGPRINTF("    matchLongUCons [%02X]\n", *graMatchPtr);

    grapheme = *myGraPtr;
	if ((grapheme & 0x80) != 0) {
		return false;
	}

	if (grapheme > 0x15) { // N
	    // above N, no match
		return false;
	}
	if (grapheme > 0x0F) { // G
	    // above G, match
		return true;
	}

	if (context==1) {
		// right context
		if ((grapheme != 0x0B) && (grapheme != 0x0C) && (grapheme != 0x0D)) { // T, S, C
			// not T, S, C -> no match
			return false;
		}
		myGraPtr++;
		grapheme = *myGraPtr;
		if ((grapheme & 0x80) != 0) {
			return false;
		}
		if (grapheme != 0x0A) { // H
		    return false;
		}
		graMatchPtr = myGraPtr;
		return true;
	} else {
		// left context
		if ((grapheme == 0x0B) || (grapheme == 0x0C)) { // T, S
			return true;
		}
		if (grapheme != 0x0A) { // H
		    return false;
		}
		myGraPtr--;
		grapheme = *myGraPtr;
		if ((grapheme & 0x80) != 0) {
			// start of word, no match
			return false;
		}
		if ((grapheme == 0x0B) || (grapheme == 0x0C) || (grapheme == 0x0D)) { // T, S, C
            graMatchPtr = myGraPtr;
			return true;
		}
		return false;
	}
}

bool Converter::matchNonLetterS()
{
	uint8_t *myGraPtr = graMatchPtr;
	uint8_t grapheme;

	DEBUGPRINTF("    matchNonLetterS [%02X]\n", *graMatchPtr);

    grapheme = *myGraPtr;
	if (grapheme > 0x19) { // non-letter
	    return true;
	}
	if (grapheme != 0x0C) { // S
	    return false;
    }

	myGraPtr++;
	grapheme = *myGraPtr;
	if (grapheme > 0x19) {
		graMatchPtr = myGraPtr;
		return true;
	}
	return false;
}

bool Converter::matchNumeric()
{
	uint8_t *myGraPtr = graMatchPtr;
	uint8_t grapheme;

	DEBUGPRINTF("    matchNumeric [%02X]\n", *graMatchPtr);

    grapheme = *myGraPtr;
	if (grapheme < '0') {
		return false;
	}
	if (grapheme > '9') {
		return false;
	}
	return true;
}

void Converter::emitPhoneme(uint8_t phoneme)
{
}

void Converter::putCharacter(char ch)
{
}

void Converter::DebugPrintf(const char *format, ...)
{
}

void Converter::DebugRule(const char *txt, const uint8_t *myRulePtr, uint8_t *graPtr)
{
}
